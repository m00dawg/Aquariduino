/*
  Aquariduino v1.23
 By: Tim Soderstrom
 
 Pins used:
 Heater Relay: A0
 Lamp Relay: A1
 LCD Shield: Analog Pins 4 & 5 (I2C Bus)
 Temperature 1 Wire Bus: 7
 */
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

/* For adding in the Ethernet or WiFi Shield */
#include <SPI.h>
#include <Ethernet.h>
//#include <WiFi.h>

/* For Timekeeping using NTP */
#include <Time.h>
#include <TimeAlarms.h>
#include <EthernetClient.h>

/* Custom Functions */
#include "devices.h"

/*
 * -------
 * DEFINES
 * ------- 
 */

/* LCD Colors */
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

/* States */
#define ON 1
#define OFF 0


/*
 * ---------
 * CONSTANTS
 * ---------
 */
const int msInSecond = 1000; //1000 ms = 1 msInSecond
const int ntpPacketSize = 48; // NTP time stamp is in the first 48 bytes of the message

/* 
 * -----------
 * CONFIG OPTS
 * -----------
 */

/* Pins */
const int temperatureProbes = 7;
const int heaterPin = A0;
const int lightPin = A1;

/* LCD */
const int lcdColumns = 16;
const int lcdRows = 2;
const int maxPage = 5;

/* Polling and update timeouts */
const int sensorPollingInterval = 5;
const int lcdUpdateInterval = 5;
const int alertTimeout = 5;

/* Web Output Options */
const boolean multiLineOutput = true;

/* 
 Temperature range to cycle heater in Celsius
 lowTemp = Temp reached before heater turns on
 highTemp = Temp reached before heater turns off
 alertHighTemp = Tank too hot even with heater off
 alertLowTemp = Tank too cold even with heater on
 */
const float lowTemp = 24.25;
const float highTemp = 24.5;
const float alertHighTemp = 27.0;
const float alertLowTemp = 23.0;

/* MAC and IP Addresss of Arduino */
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 
  192, 168, 100, 11 };

/* NTP Server ^ Time Handling */
uint16_t ntpPort = 123;
unsigned int ntpLocalPort = 8888;
byte ntpServer[] = { 
  192, 168, 100, 2 };
const long timeZoneOffset = -21600; // UTC-6

/*
 * -------
 * OBJECTS
 * ------- 
 */

// LCD Object
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(temperatureProbes);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Arrays to hold temperature devices 
// DeviceAddress insideThermometer, outsideThermometer;
DeviceAddress tankThermometer;

EthernetServer webServer(80);
EthernetClient webClient;
EthernetUDP udp;

DeviceOnSchedule light = 
{
  { 8, 0, 21, 0 }, // 8am - 9pm CST
  lightPin,                    // Analog Pin 1
  false                 // State
};            

/*
 * ----------------
 * STATUS VARIABLES
 * ----------------
 */

/* Number of millimsInSeconds since bootup */
unsigned long currentMillis = 0;

/* Poll Timeouts for various things */
unsigned long lastSensorPoll = 0;
unsigned long lastLCDUpdate = 0;

/* Min and max temperatures seen */
float maxTemp = 0;
float minTemp = 100;
float currentTemp = 0;

/* LCD Status Variables */
boolean clearLCD = false;
boolean backlight = true;
boolean displayPage = false;
int backlightColor = WHITE;
int page = 0;

/* Heater Status */
boolean heater = FALSE;
int heaterCycles = 0;

/* Variable to store buttons */
uint8_t buttons = 0;

/* Switches from C to F for display */
boolean displayCelsius = true;

//char serialInput = '\0';

/* NTP */
byte ntpBuffer[ntpPacketSize];
time_t time;

void setup()
{ 
  Serial.begin(9600); 
  /*
  while (!Serial)
   {
   ; // wait for serial port to connect. Needed for Leonardo only
   }
   */
  //Serial.println("Aquariduino");

  // Start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  //Start UDP for NTP
  udp.begin(ntpLocalPort);

  //Start the webserver
  webServer.begin();

  // set up the LCD's number of columns and rows: 
  lcd.begin(lcdColumns, lcdRows);

  /* Turn off header, since we don't know what's going on until
   we poll the temperature sensor */
  pinMode(light.pin, OUTPUT);
  pinMode(heaterPin, OUTPUT);
  digitalWrite(heaterPin, LOW);


  /* Print Title and Version */
  lcd.setBacklight(WHITE);
  lcd.setCursor(0,0);
  lcd.print("Aquariduino");
  lcd.setCursor(0,1);
  lcd.print("v1.23");
  delay(2000);
  lcd.clear();

  //Sync to NTP
  syncTime();
  
  // Initialize Temp Sensor Library
  sensors.begin();
}

void loop()
{
  buttons = lcd.readButtons(); 
  webClient = webServer.available();

  currentMillis = millis();

  /* If it's been longer than the polling interval, poll sensors */
  if(currentMillis - lastSensorPoll > sensorPollingInterval * msInSecond)
  {
    if(collectTemperatures())
      controlHeater();  
    else
      error("NO SENSORS");
    
    // Check to see if it's time for light to turn on/off
    checkSchedule(light);
    
    lastSensorPoll = currentMillis;
  }

  /* Process button input */
  if (buttons)
  {
    if(buttons & BUTTON_UP)
    {
      if(displayCelsius)
        displayCelsius = false;
      else
        displayCelsius = true;
      displayCurrentTemp();
      lastLCDUpdate = millis();
      clearLCD = true;
    }
    else if (buttons & BUTTON_DOWN)
    {
      syncTime();
    }
    else if (buttons & BUTTON_SELECT)
    {
      if(backlight)
      {
        lcd.setBacklight(OFF);
        backlight = false;
        delay(msInSecond);
      }
      else
      {
        lcd.setBacklight(backlightColor);
        backlight = true;
        delay(msInSecond); 
      }
    }
    else if(buttons & BUTTON_LEFT)
    {
      --page;
      if(page < 0)
        page = maxPage; 
      displayPage = true;
    }
    else if(buttons & BUTTON_RIGHT)
    {
      ++page;
      if(page > maxPage)
        page = 0;
      displayPage = true;
    }
  }
  if(displayPage)
  {
    switch(page)
    {
    case 0:
      {
        displayInfo("Min/Max Temps:", formatTemperatures(minTemp, maxTemp));
        break;
      }
    case 1:
      {
        displayInfo("Alert Temps:", formatTemperatures(alertLowTemp, alertHighTemp));
        break; 
      }
    case 2:
      {
        displayInfo("CFG Temp Range:", formatTemperatures(lowTemp, highTemp));
        break; 
      }
    case 3:
      {
        displayInfo("Uptime (Secs):", String(millis() / msInSecond));
        break;
      }
    case 4:
      {
        displayInfo("Heater Cycles:", String(heaterCycles));
        break;
      }
    case 5:
      {
        displayInfo("Time:", String(printTime(now())));
      }
    }
    lastLCDUpdate = millis();
    clearLCD = true;
    displayPage = false;
  }

  /* Regular Display Routine */
  else if(currentMillis - lastLCDUpdate > lcdUpdateInterval * msInSecond)
  {
    displayCurrentTemp();
    lastLCDUpdate = millis();
  }

  /* Web-Server */
  if (webClient)
  {
    webProcessInput();
    webPrintRawStats();
    webClient.stop();
  }
}

/*
void serialEvent()
 {
 while(Serial.available() > 0)
 serialInput = Serial.read(); 
 if(serialInput == 'P')
 {
 serialInput = '\0';
 Serial.print("WaterTemp:");
 Serial.print(currentTemp, DEC);
 Serial.print(" Heater:");
 Serial.println(heater, DEC);
 }  
 }
 */

