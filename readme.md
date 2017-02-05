BeeMonitor
==========

Arduino and Raspberry Pi Source Code for Bee Hive Temperature Monitoring Project http://beemonitor.org/setup

***

* **Arduino Tx** - read from DS18B20 temperature sensors every 60's, read battey voltage and and tramsmitt via RF. Atmega328 is put to sleep in between readings to save battery power 

* **Arduino Rx** - Receive data via RF, decod and print to serial - Arduino connected to web-connected Raspberry Pi via USB serial 


* Raspberry Pi uses emonHub serial listener to receive data via USB serial port (/dev/ttyUSB0) and post to https://emoncms.org for loggin and graphing 

emonHub: http://github.com/openenergymonitor/emonhub

Blog Post with instructions for using emonHub Serial Listner to post to emoncms: https://blog.openenergymonitor.org/2014/01/oem-gateway-serial-port-emoncms-link/

Forum thread: http://openenergymonitor.org/emon/node/3693#comment-17847


Part of the http://OpenEnergyMonitor.org project 

Bee Monitoring Project: http://beemonitor.org

Glyn Hudson & Clive Hudson
