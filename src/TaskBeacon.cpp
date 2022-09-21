#include <logger.h>

#include <OneButton.h>
#include <TimeLib.h>

#include "Task.h"
#include "TaskBeacon.h"
#include "project_configuration.h"

#include  "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//define port for DHT sensor
DHT dhtobj(25,DHT22);

//define port for DS18B20 port and sensor address. This cannot be done during runtime.
OneWire oneWire(4);
DallasTemperature sensors(&oneWire);
DeviceAddress DS18Address;

BeaconTask::BeaconTask(TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs) : Task(TASK_BEACON, TaskBeacon), _toModem(toModem), _toAprsIs(toAprsIs), _ss(1), _useGps(false) {
}

BeaconTask::~BeaconTask() {
}

OneButton BeaconTask::_userButton;
bool      BeaconTask::_send_update;
uint      BeaconTask::_instances;


// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


void BeaconTask::pushButton() {
  _send_update = true;
}

bool BeaconTask::setup(System &system) {

  
  if (_instances++ == 0 && system.getBoardConfig()->Button > 0) {
    _userButton = OneButton(system.getBoardConfig()->Button, true, true);
    _userButton.attachClick(pushButton);
    _send_update = false;
  }
  
  //initialize DHT sensor
  dhtobj.begin();

  //initialize DS180B20 sensor
  sensors.begin();
  
  
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount());
  Serial.println(" devices.");
  
  //read hardware address for first found DS18B20 sensor
  sensors.getAddress(DS18Address, 0);
  printAddress(DS18Address);
  
  _useGps = system.getUserConfig()->beacon.use_gps;
 
  
  if (_useGps) {
    if (system.getBoardConfig()->GpsRx != 0) {
      _ss.begin(9600, SERIAL_8N1, system.getBoardConfig()->GpsTx, system.getBoardConfig()->GpsRx);
    } else {
      system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "NO GPS found.");
      _useGps = false;
    }
  }
  // setup beacon
  _beacon_timer.setTimeout(system.getUserConfig()->beacon.timeout * 60 * 1000);

  //define message for beacon over internet 
  _beaconMsg = std::shared_ptr<APRSMessage>(new APRSMessage());
  _beaconMsg->setSource(system.getUserConfig()->callsign);
  _beaconMsg->setDestination("APLG01");

  //define message for telemetry beacon over rf 
  _TeleBeaconMsg = std::shared_ptr<APRSMessage>(new APRSMessage());
  _TeleBeaconMsg->setSource(system.getUserConfig()->telemetry.telemetry_call);
  _TeleBeaconMsg->setDestination("APLG01");

  return true;
}

bool BeaconTask::loop(System &system) {
  if (_useGps) {
    while (_ss.available() > 0) {
      char c = _ss.read();
      _gps.encode(c);
    }
  }

  _userButton.tick();

  // check for beacon
  if (_beacon_timer.check() || _send_update) {
    if (sendBeacon(system)) {
      _send_update = false;
      _beacon_timer.start();
    }
  }

  uint32_t diff = _beacon_timer.getTriggerTimeInSec();
  _stateInfo    = "beacon " + String(uint32_t(diff / 600)) + String(uint32_t(diff / 60) % 10) + ":" + String(uint32_t(diff / 10) % 6) + String(uint32_t(diff % 10));

  return true;
}

String create_lat_aprs(double lat) {
  char str[20];
  char n_s = 'N';
  if (lat < 0) {
    n_s = 'S';
  }
  lat = std::abs(lat);
  sprintf(str, "%02d%05.2f%c", (int)lat, (lat - (double)((int)lat)) * 60.0, n_s);
  String lat_str(str);
  return lat_str;
}

String create_long_aprs(double lng) {
  char str[20];
  char e_w = 'E';
  if (lng < 0) {
    e_w = 'W';
  }
  lng = std::abs(lng);
  sprintf(str, "%03d%05.2f%c", (int)lng, (lng - (double)((int)lng)) * 60.0, e_w);
  String lng_str(str);
  return lng_str;
}

/*
  Reads out temperature from a DS18B20 sensor
  Returns results in string-format
*/ 

String getTempDS18(System &system){
  char outBuffer[40]; 
  
  float t;
  sensors.requestTemperatures();
  t = sensors.getTempC(DS18Address);
  
  sprintf(outBuffer, "T2: %3.1fC ",t);
  
  return (String) outBuffer;  
}


/*
  Reads out temperature and humidity values form a dht22 sensor on io pin
  configured.
  Returns results in string-format
*/

String getTempHumid(System &system){
  char outBuffer[40]; 
  //get IO port to read a dht22 from configuration
  //int dhtPort = system.getUserConfig()->telemetry.dht22_pin;
  float h = dhtobj.readHumidity();
  float t = dhtobj.readTemperature();
  
  sprintf(outBuffer, "h: %3.1f%%, T1: %3.1fC ",h,t);
  return (String) outBuffer;  
}

/*
  Collect and format measurements connected to IO Ports at the
  local board and format it into a string to be sent out as telemetry data.
  These can be e.g. direct voltage measurements from 0-3.3V, connected 
  temperature sensors, etc. 
*/



String getVoltage(System &system){
 char outBuffer[40]; 
  String tmpOutStr="";

  //get IO port to read a direct voltage from configuration
  int voltagePort = system.getUserConfig()->telemetry.voltage_pin;

  //get voltage scaling faktor. This is used for output formatting and gives the real world voltage value of an IO pin input of 3.3V 
  float voltageScaling = system.getUserConfig()->telemetry.voltage_scaling;

  //do an average over some measuremets to reduce jitter
  int v=0;
  for (int i=0;i<5;i++){
    v+=analogRead(voltagePort);
  }
  v=v/5;

  // analog ports act not linear, so we have to do some linearisation
  double vf=-0.15675368+0.00114342*v-0.00000008*v*v;
  //now scale to configured real world max value
  vf=vf/3.3*voltageScaling;
  //voltage reading must be >=0, if negative values occur, these are up to the linearization function, so set them to 0
  if (vf<0) {
    vf=0;
  } 

  //format for proper readibility
  sprintf(outBuffer, "U: %3.1fV ", vf);
  tmpOutStr=(String) outBuffer;
  return (String) outBuffer;  
}  

String getTelemetryData(System &system){
  char outBuffer[40]; 
  String tmpOutStr="";
  tmpOutStr=tmpOutStr+getVoltage(system)+" "+getTempHumid(system)+" "+getTempDS18(system);
  system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "","Telemetriedaten: %s", tmpOutStr);
  
  return (String) tmpOutStr;  
}


bool BeaconTask::sendBeacon(System &system) {
  double lat = system.getUserConfig()->beacon.positionLatitude;
  double lng = system.getUserConfig()->beacon.positionLongitude;

  if (_useGps) {
    if (_gps.location.isUpdated()) {
      lat = _gps.location.lat();
      lng = _gps.location.lng();
    } else {
      return false;
    }
  }
  _beaconMsg->getBody()->setData(String("=") + create_lat_aprs(lat) + "L" + create_long_aprs(lng) + "&" + system.getUserConfig()->beacon.message);
  _TeleBeaconMsg->getBody()->setData(String("=") + create_lat_aprs(lat) + "L" + create_long_aprs(lng) + "&" + system.getUserConfig()->beacon.message);

  system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "[%s] %s", timeString().c_str(), _beaconMsg->encode().c_str());
  system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "[%s] %s", timeString().c_str(), _TeleBeaconMsg->encode().c_str());


  if (system.getUserConfig()->aprs_is.active) {
    _beaconMsg->setSource(system.getUserConfig()->callsign);
    _toAprsIs.addElement(_beaconMsg);
    //Serial.print("toAPRS Message gesetzt");
  }

  if (system.getUserConfig()->digi.beacon) {
    _beaconMsg->setSource(system.getUserConfig()->callsign);
    _toModem.addElement(_beaconMsg);
  }

  if (system.getUserConfig()->telemetry.active) {
    //_TeleBeaconMsg->setSource(system.getUserConfig()->telemetry.telemetry_call);
    _TeleBeaconMsg->setPath("WIDE1-1");
    _TeleBeaconMsg->setType('>');
    _TeleBeaconMsg->getBody()->setData(String("=") + create_lat_aprs(lat) + "L" + create_long_aprs(lng) + "& "  + getTelemetryData(system));
     _toModem.addElement(_TeleBeaconMsg);
  
  }

  system.getDisplay().addFrame(std::shared_ptr<DisplayFrame>(new TextFrame("BEACON", _beaconMsg->toString())));

  return true;
}

