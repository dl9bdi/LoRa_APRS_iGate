#ifndef TASK_BEACON_H_
#define TASK_BEACON_H_

#include <OneButton.h>
#include <TinyGPS++.h>

#include <APRSMessage.h>
#include <TaskMQTT.h>
#include <TaskManager.h>



class BeaconTask : public Task {
public:
  BeaconTask(TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs);
  virtual ~BeaconTask();

  virtual bool setup(System &system) override;
  virtual bool loop(System &system) override;
  bool         sendBeacon(System &system);
  String getTelemetryData(System &system);
  

private:
  TaskQueue<std::shared_ptr<APRSMessage>> &_toModem;
  TaskQueue<std::shared_ptr<APRSMessage>> &_toAprsIs;

  std::shared_ptr<APRSMessage> _beaconMsg;      //message object for beacon messages
  std::shared_ptr<APRSMessage> _TeleBeaconMsg;  //message object for telemetry data, not to mess it up with standard object

  Timer                        _beacon_timer;

  HardwareSerial _ss;
  TinyGPSPlus    _gps;
  bool           _useGps;

  static uint      _instances;
  static OneButton _userButton;
  static bool      _send_update;
  static void      pushButton();
  //static uint     _telemetrySequence; // sequence counter for sending out telemetry beacon
  
  


};

#endif
