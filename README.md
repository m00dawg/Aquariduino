Aquariduino
===========

Arduino application To Track and Control Aquarium Parameters

Hardware Requirements:

- Arduino Uno (most compatibles should work)
- 2x - Relay/AC Switch (PowerSwitch Tail II)
- Waterproof Temperature Sensor (DS18B20)
- Adafruit's RGBLCDShield (or something that behaves like it)

Libraries:

- OneWire (http://www.pjrc.com/teensy/td_libs_OneWire.html)
- DallasTemperature (http://milesburton.com/Dallas_Temperature_Control_Library)
- Adafruit RGB LCD Shield (https://github.com/adafruit/Adafruit-RGB-LCD-Shield-Library)
- UPDbitewise Library (https://bitbucket.org/bjoern/arduino_osc/get/tip.zip)


Pins Used (Defaults):

- Digital Pin 12: Relay to Control Aquarium Heater
- Digital Pin 7: Interface to 1-Wire Temperature Sensor
- Analog Pins 4,5: LCD Shield (I2C Bus)

Assembly:

For a proper build-guide, go here:

http://www.moocowproductions.org/articles/aquariduino/
