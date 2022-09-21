# LoRa APRS iGate with rf telemetry beacon

Peter Buchegger, OE5BPA, did a great job in providing software for LoRa devices so that they could be used as 
as iGate or Tracker. This project is a fork of Peter's LoRa iGate project to add the functionality of a radio beacon.

iGates often operate autonomously at locations such as a repeater site, which acts as a gateway for other LoRa stations 
such as trackers to aprs.fi to present location information or other APRS data.
The path of the data goes from RF side to the Internet.

It would be great if such an iGate would also be able to send status information about the repeater location via radio.
This data could then be received by other LoRa iGate stations and displayed on aprs.fi.

The idea behind this is to have a decoupled information channel to monitor the repeater equipment and environment. Even if the 
internet connection to the repeater site fails and remote access is not possible, one can at least see if the repeater is working in general.

If the iGate hardware is buffered by a bettery one could even detect power outages at the repeater site.

This project contains modifications to Peters code that realize such a radio beacon. It reads out several sensors connected to the iGate ESP board
* one DHT22 sensor to measure humidity and temperature at the site
* one DS18B20 sensor e.g. to be placed at transmitters pa to measure special hardware temperatures
* one direct voltage measurement pin to read out the sites power supply

The measures values are send out by the radio module of the iGate as regular APRS message and can be viewed on aprs.fi.

It is possible to configure a seperate outgoing call (e.g. CALL-2) to be able to destinguish between iGate and Telemetry

Up to know this version ha a protoype status and has to be optimized and refined - but it is working for me.

Below you can find the original documentation from OE5BPA's project.

vy73 de Matthias, DL9BDI


---

The LoRa APRS iGate will work with very cheep hardware which you can buy from amazon, ebay or aliexpress.
Try it out and be part of the APRS network.

![TTGO LoRa32](pics/iGate.png)

## Blog posts and Youtube videos from other Hams

* [Manuel Lausmann - iGate & Tracker](https://www.youtube.com/watch?v=-KdFQEaNC1k) (youtube - german) 04.06.2021
* [Manuel Lausmann - Installationsanleitung als PDF](http://mala-one.de/Lora-APRS/) (PDF - german) 04.06.2021
* [OE1ROT](https://www.aronaut.at/2020/11/lora-aprs-gateway-mit-esp32-boards/) (blog post - german) 14.11.2020
* [DL7AG](http://online.dl7ag.de/lora-aprs-dl7ag-10/) (blog post - german) 08.11.2020
* [Manuel Lausmann - iGate](https://www.youtube.com/watch?v=C7hfVe32pXs) (youtube - german - OLD) 06.11.2020
* [Manuel Lausmann - Tracker](https://www.youtube.com/watch?v=clIlTEFbWLk) (youtube - german - OLD) 02.11.2020
* [OE1ROT](https://www.aronaut.at/2019/12/lora-aprs-tracker-mit-ttgo-t-beam-433mhz/) (blog post - german) 09.12.2019

feel free  to add yours or create a ticket if you want to be added.

## Supported boards

You can use one of the Lora32 boards without changings:

* Heltec WiFi LoRa 32 V1 (433MHz SX1278)
* Heltec WiFi LoRa 32 V2 (433MHz SX1278)
* TTGO LoRa32 V1 (433MHz SX1278)
* TTGO LoRa32 V2 (433MHz SX1278)
* TTGO LoRa32 V2.1 (433MHz SX1278)
* TTGO T-Beam V0.7 (433MHz SX1278)
* TTGO T-Beam V1 (433MHz SX1278)
* Tracker D from [OE1ACM and OE1CGC](https://www.lora-aprs.at/)
* and sure many more...

Here are some amazon-de links for some example boards:
* [LoRa32 V1](https://www.amazon.de/dp/B07VPHYYJD)
* [LoRa32 V1](https://www.amazon.de/dp/B07QRG89ZV)
* [LoRa32 V2](https://www.amazon.de/dp/B07VL97VNH)
* [LoRa32 V2.1](https://www.amazon.de/dp/B07RXSKPBX)
* [T-Beam V1.0](https://www.amazon.de/dp/B07RT9FKPL)

This boards cost around 20 Euros, they are very cheap and perfect for an LoRa iGate.
Keep in minde: you need a 433MHz version!

## Compiling and configuration

**There is a german [quick start](https://www.lora-aprs.info/docs/LoRa_APRS_iGate/quick-start-guide/) page! Take a look ;)**

**There is a french [quick start](http://www.f5kmy.fr/spip.php?article509) page! Take a look ;)**

### How to compile

The best success is to use PlatformIO (and it is the only platform where I can support you). 

* Go to [PlatformIO](https://platformio.org/) download and install the IDE. 
* If installed open the IDE, go to the left side and klick on 'extensions' then search for 'PatformIO' and install.
* When installed click 'the ant head' on the left and choose import the project on the right.
* Just open the folder and you can compile the Firmware.

### Configuration

* You can find all nessesary settings to change for your configuration in **data/is-cfg.json**.
* To upload it to your board you have to do this via **Upload File System image** in PlatformIO!
* To find the 'Upload File System image' click the PlatformIO symbol (the little alien) on the left side, choos your configuration, click on 'Platform' and search for 'Upload File System image'.

## Branches in this repository and version system

This firmware is developed in a rolling release system: everyday a new release could be created. But there are still rules where new pull requests has to go and and how the version system looks like.

### Version system

If the *develop* branch is stable enough for a new release it will be merged with a pull request to the *master* branch and a new version will be generated.

The versions are based on this settings:
* major: the current year (2 digits)
* minor: the current week of the year
* patch: if there is a important fix for an release, just increment the number, otherwise 0

*example*: a new release will be created on the 11/14/2020, this version numbers will be used: 
* major: 20
* minor: 46
* patch: 0

so the version will be: 20.46.0

## Future plans

* [x] show time until next beaconing
* [ ] show login issues from IS server
* [ ] add better OLED library to support multiple different OLEDs
* [x] add support to turn OLED on, off and dimming
* [ ] add support for temperature chips (BMExxx)
* [x] add FTP server support to upload configuration
* [ ] add web server for configuration and other things
* [ ] add statistics for received packages
* [ ] show received packages on a map
* [ ] etc.

## LoRa Tracker

Look at my other project: a [LoRa Tracker](https://github.com/peterus/LoRa_APRS_Tracker)

## Hints

### Here are some peculiarities of the different boards

* TTGO T-Beam V1

When adding a 0,96" OLED display direct to the board you have to be careful, there are two different pinout
versions on the market. 
For direct mount you need a display with this Pinout -> [VCC - GND - SCL - SDA](pics/display-right.jpg).
A direct mount of the [other display](pics/display-wrong.jpg) is not possible without damage the display!
The 'wrong' display works too but you have to change VCC and GND by wire !

feel free to add hints!

