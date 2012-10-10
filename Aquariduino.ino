/*
  Aquariduino v0.25
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
//#include <SPI.h>
//#include <Ethernet.h>
//#include <WiFi.h>

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
 
const int second = 1000;

/* LCD */
const int lcdColumns = 16;
const int lcdRows = 2;
 
 /* Pins */
const int temperatureProbes = 7;
const int heaterPin = A0;

/* Polling and update timeouts */
const int sensorPollingInterval = 5;
const int lcdUpdateInterval = 5;
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


/*
 * ----------------
 * STATUS VARIABLES
 * ----------------
*/

/* Number of milliseconds since bootup */
unsigned long currentMillis = 0;

/* Poll Timeouts for various things */
unsigned long lastSensorPoll = 0;
unsigned long lastLCDUpdate = 0;

/* Min and max temperatures seen */
float maxTemp = 0;
float minTemp = 100;
float currentTemp = 0;

/* LCD Status Variables */
boolean clearLCD = FALSE;
boolean backlight = TRUE;
int backlightColor = WHITE;

/* Heater Status */
boolean heater = FALSE;

/* Variable to store buttons */
uint8_t buttons = 0;

void setup()
{
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
  lcd.print("v0.25");
  delay(2000);
  lcd.clear();
  
  // Initialize Temp Sensor Library
  sensors.begin();
}

void loop()
{
  buttons = lcd.readButtons(); 

  currentMillis = millis();
  
  /* If it's been longer than the polling interval, poll sensors */
  if(currentMillis - lastSensorPoll > sensorPollingInterval * second)
  {
    if(collectTemperatures())
      controlHeater();  
    else
      error("NO SENSORS");
    lastSensorPoll = currentMillis;
  }
  
  /* Process button input */
  if (buttons)
  {
    if (buttons & BUTTON_LEFT)
      displayInfo("Temp Range:", String(floatToString(lowTemp)) + "-" + String(floatToString(highTemp)) + " C");
    if (buttons & BUTTON_RIGHT)
      displayInfo("Alert Temps:", String(floatToString(alertLowTemp)) + "-" + String(floatToString(alertHighTemp)) + " C");
    if (buttons & BUTTON_UP)
      displayInfo("Min/Max Temps:", String(floatToString(minTemp)) + "-" + String(floatToString(maxTemp)) + " C");
    if (buttons & BUTTON_DOWN)
      displayInfo("Uptime (Secs):", String(millis() / second));
    if (buttons & BUTTON_SELECT)
    {
      if(backlight)
      {
        lcd.setBacklight(OFF);
        backlight = false;
        delay(second);
      }
      else
      {
        lcd.setBacklight(backlightColor);
        backlight = true;
        delay(second);   
      }
    }
    lastLCDUpdate = millis();
    clearLCD = true;
  }
  
  /* Regular Display Routine */
  else if(currentMillis - lastLCDUpdate > lcdUpdateInterval * second)
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
    if(heater)
      displayInfo("Temp: " + String(floatToString(currentTemp)) + " C", "Heater On");
    else
      displayInfo("Temp: " + String(floatToString(currentTemp)) + " C", "Heater Off");
    lastLCDUpdate = millis();
  }
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
      heater = true;
    }
}

void error(String message)
{
    displayInfo("**** ERROR ****", message);
    for(int count = 0; count < alertTimeout; ++count)
    {
      lcd.setBacklight(RED);
      delay(second);
      lcd.setBacklight(BLUE);
      delay(second);
    }
}
