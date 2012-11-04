/*
  Aquariduino v1.10
  By: Tim Soderstrom

  Pins used:
    Heater Relay: A0
    LCD Shield: Analog Pins 4 & 5 (I2C Bus)
    Temperature 1 Wire Bus: 2
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

/* For NTP */
#include <Time.h> 
#include <EthernetClient.h>

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
const int ntpSyncInterval = 60;
const int alertTimeout = 5;

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

/* 
  Light Time Range
  Start Hour, Start Minute, Stop Hour, Stop Minute
*/
const int lightSchedule[] = { 15, 00, 02, 00 }; //9am - 9pm CST

/* MAC and IP Addresss of Arduino */
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 100, 11 };

/* NTP Server */
uint16_t ntpPort = 123;
unsigned int ntpLocalPort = 8888;
byte ntpServer[] = { 192, 168, 100, 2 };


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

/* Light Status */
boolean light = FALSE;

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
  Serial.println("Aquariduino");
  
  // Start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  
  //Start the webserver
  webServer.begin();

  // set up the LCD's number of columns and rows: 
  lcd.begin(lcdColumns, lcdRows);

  /* Turn off header, since we don't know what's going on until
      we poll the temperature sensor */
  pinMode(heaterPin, OUTPUT);
  digitalWrite(heaterPin, LOW);

  /* Print Title and Version */
  lcd.setBacklight(WHITE);
  lcd.setCursor(0,0);
  lcd.print("Aquariduino");
  lcd.setCursor(0,1);
  lcd.print("v1.10");
  delay(2000);
  lcd.clear();
  
  // Set the time sync source to NTP and sync
  lcd.setCursor(0,0);
  lcd.print("Syncing Time");
  Serial.println("Syncing Time");
  setSyncProvider(getUnixTimeFromNTP);
  //while(timeStatus() == timeNotSet)
  //  ; //Twiddle Thumbs
  // Set how often clock is synced
  setSyncInterval(ntpSyncInterval * msInSecond); 
  
  
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
    lastSensorPoll = currentMillis;
  }

  /* Lights */  
  controlLight();

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
       // displayInfo("Min/Max Temps:", String(floatToString(minTemp)) + "-" + String(floatToString(maxTemp)) + " C");
        displayInfo("Min/Max Temps:", formatTemperatures(minTemp, maxTemp));
        break;
      }
      case 1:
      {
        //displayInfo("Alert Temps:", String(floatToString(alertLowTemp)) + "-" + String(floatToString(alertHighTemp)) + " C");
        displayInfo("Alert Temps:", formatTemperatures(alertLowTemp, alertHighTemp));
        break; 
      }
      case 2:
      {
        //displayInfo("Temp Range:", String(floatToString(lowTemp)) + "-" + String(floatToString(highTemp)) + " C");
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

void webProcessInput()
{
  char character;
  boolean currentLineIsBlank = true;
  while (webClient.connected())
  {
    if(webClient.available())
    {
      character = webClient.read();
      if (character == '\n' && currentLineIsBlank)
        return;
      if (character == '\n')
        currentLineIsBlank = true; 
      else if (character != '\r')    
        currentLineIsBlank = false;
    }
    
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

void webPrintRawStats()
{
    webClient.println("Temp:" + String(floatToString(currentTemp)) + " Heater:" + heater + " Light: " + light);
}

String formatTemperature(float temperature)
{
  if(displayCelsius)
   return String(floatToString(temperature)) + "C";
  return String(floatToString(sensors.toFahrenheit(temperature))) + "F";
}

String formatTemperatures(float temp1, float temp2)
{
  if(displayCelsius)
   return String(floatToString(temp1)) + "-" + String(floatToString(temp2)) + " C";
  return String(floatToString(sensors.toFahrenheit(temp1)))
    + "-" + String(floatToString(sensors.toFahrenheit(temp2))) + "F"; 
}

void displayCurrentTemp()
{
  if(backlight)
  {
    if(currentTemp > alertHighTemp)
      lcd.setBacklight(RED);
    else if(currentTemp >= highTemp)
      lcd.setBacklight(YELLOW);
    else if(currentTemp > lowTemp && currentTemp < highTemp)
      lcd.setBacklight(GREEN);
    else if(currentTemp <= lowTemp)
      lcd.setBacklight(VIOLET);
    else if(currentTemp < alertLowTemp)
      lcd.setBacklight(BLUE);
  }
  /*
  if(heater)
    displayInfo("Temp: " + formatTemperature(currentTemp), "Heater On");
  else
    displayInfo("Temp: " + formatTemperature(currentTemp), "Heater Off"); 
  */
  displayInfo("Temp: " + formatTemperature(currentTemp), 
    "H:" + (String)heater + " L:" + (String)light);
}


void displayInfo(String topText, String bottomText)
{  
  if(clearLCD)
  {
    lcd.clear();
    clearLCD = false;
  }
  lcd.setCursor(0,0);
  lcd.print(padString(topText));
  lcd.setCursor(0,1);
  lcd.print(padString(bottomText));
}

String padString(String value)
{
  char padding[lcdColumns - value.length()];
  for(int count = 0; count < sizeof(padding); ++count)
    padding[count] = ' ';
  return String(value) + String(padding);
}

String floatToString(float value)
{
  char result[13];
  dtostrf(value, 4, 1, result);
  return result;
}

boolean collectTemperatures()
{
  sensors.requestTemperatures();
  if(sensors.getAddress(tankThermometer, 0))
  {
    currentTemp = sensors.getTempC(tankThermometer);
    /* Check to see if we hit a new low or high temp */
    if(currentTemp > maxTemp)
      maxTemp = currentTemp;
    if(currentTemp < minTemp)
      minTemp = currentTemp;
    return true;
  }
  return false;
}

void controlHeater()
{
    /* If temperature is too high, turn off heater */
    if(currentTemp > highTemp)
    {
      digitalWrite(heaterPin, LOW);
      heater = false;
      return;
    }
    /* If the temperature is too low, turn on heater */
    if(currentTemp < lowTemp)
    {
      digitalWrite(heaterPin, HIGH);
      if(heater)
        ++heaterCycles;
      heater = true;
    }
}

void controlLight()
{
  /* Light Schedule */
  if(hour(now()) >= lightSchedule[0] & 
     minute(now()) >= lightSchedule[1] &
     hour(now()) <= lightSchedule[2] & 
     minute(now()) <= lightSchedule[3])
  {
    digitalWrite(lightPin, HIGH);
    light = true;
  }
  else
  {
    digitalWrite(lightPin, LOW);
    light = false;
  }
}

void error(String message)
{
    displayInfo("**** ERROR ****", message);
    for(int count = 0; count < alertTimeout; ++count)
    {
      lcd.setBacklight(RED);
      delay(msInSecond);
      lcd.setBacklight(BLUE);
      delay(msInSecond);
    }
}

unsigned long getUnixTimeFromNTP()
{
  const unsigned long seventy_years = 2208988800UL;
  unsigned long highWord, lowWord, epoch;

  Serial.println('Setting NTP Time');  
  udp.begin(ntpLocalPort);
  sendNTPPacket(ntpServer, ntpPort);
  delay(msInSecond);
  
  if(udp.parsePacket())
  {
    udp.read(ntpBuffer, ntpPacketSize);
    highWord = word(ntpBuffer[40], ntpBuffer[41]);
    lowWord = word(ntpBuffer[42], ntpBuffer[43]);  
    epoch = highWord << 16 | lowWord;
    epoch = epoch - seventy_years;
    Serial.println("Time is: " + epoch);
    return epoch;
  }
  return 0; // return 0 if unable to get the time
}

unsigned long sendNTPPacket(byte *address, uint16_t port)
{
  // set all bytes in the buffer to 0
  memset(ntpBuffer, 0, ntpPacketSize);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  ntpBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntpBuffer[1] = 0;     // Stratum, or type of clock
  ntpBuffer[2] = 6;     // Polling Interval
  ntpBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntpBuffer[12]  = 49;
  ntpBuffer[13]  = 0x4E;
  ntpBuffer[14]  = 49;
  ntpBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  udp.beginPacket(address, port); //NTP requests are to port 123
  udp.write(ntpBuffer,ntpPacketSize);
  udp.endPacket(); 
}

String printTime(time_t time)
{
  return (String)hour(time) + ":" + (String)minute(time) + ":" + (String)second(time);
}

