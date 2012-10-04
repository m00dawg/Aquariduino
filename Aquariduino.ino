/*
  Aquariduino v0.20

  Pins used:
    Heater Relay: 12
    LCD Shield: Analog Pins 4 & 5 (I2C Bus)
    Temperature 1 Wire Bus: 7
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

/* One wire bus pin */
#define ONE_WIRE_BUS 7

/* Initialize LCD object */
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Arrays to hold temp devices 
// DeviceAddress insideThermometer, outsideThermometer;
DeviceAddress tankThermometer;

/* Define LCD Size */
const int lcdColumns = 16;
const int lcdRows = 2;

/* Define Fridge AC Relay Control Pin */
const int heaterPin = 12;

/* 1 second = 1000 milliseconds */
const int second = 1000;

/* Temp sesnor polling interval (seconds) and display hold time */
const int pollingInterval = 5;

/* 
   Temperature range to cycle heater in Celsius
   lowTemp = Temp reached before heater turns on
   highTemp = Temp reached before heater turns off
   alertHighTemp = Tank too hot even with heater off
   alertLowTemp = Tank too cold even with heater on
*/
const float lowTemp = 24.5;
const float highTemp = 25.0;
const float alertHighTemp = 27.0;
const float alertLowTemp = 23.0;

/*
 * ----------------
 * STATUS VARIABLES
 * ----------------
*/

/* Number of milliseconds since bootup */
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

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

  /* Get some temperature readings and do stuff */
  currentMillis = millis();
    
  if(currentMillis - previousMillis > pollingInterval * second)
  {
    previousMillis = currentMillis;    
    sensors.requestTemperatures();
    if(sensors.getAddress(tankThermometer, 0))
    {
      currentTemp = sensors.getTempC(tankThermometer);
      /* Check to see if we hit a new low or high temp */
      if(currentTemp > maxTemp)
        maxTemp = currentTemp;
      if(currentTemp < minTemp)
        minTemp = currentTemp;
      /* If temperature is too high, turn off heater */
      if(currentTemp > highTemp)
      {
        digitalWrite(heaterPin, LOW);
        heater = false;
        if(currentTemp > alertHighTemp)
          backlightColor = RED;
        else
          backlightColor = GREEN;
      }
      
      /* If the temperature is too low, turn on heater */
      else if(currentTemp < lowTemp)
      {
        digitalWrite(heaterPin, HIGH);
        heater = true;
        if(backlight)
        {
          if(currentTemp < alertLowTemp)
            backlightColor = BLUE;
          else
            backlightColor = YELLOW;
        }
      }
      if(heater)
        displayInfo("Temp: " + String(floatToString(currentTemp)) + "C", "Heater On");
      else
        displayInfo("Temp: " + String(floatToString(currentTemp)) + "C", "Heater Off");  
    }
    else
    {
      backlightColor = RED;
      displayInfo("NO TEMP SENSORS", "");
    }
    if(backlight)
      lcd.setBacklight(backlightColor);
  }
  
  /* Process button input */
  if (buttons)
  {
    if (buttons & BUTTON_LEFT)
      displayInfo("CFG Temp Range:", String(floatToString(lowTemp)) + "-" + String(floatToString(highTemp)) + " C");
    if (buttons & BUTTON_RIGHT)
      displayInfo("Temp Alerts:", String(floatToString(alertLowTemp)) + "-" + String(floatToString(alertHighTemp)) + " C");
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
        delay(1 * second);
      }
      else
      {
        lcd.setBacklight(ON);
        lcd.setBacklight(backlightColor);
        backlight = true;
        delay(1 * second);   
      }
    }
    previousMillis = millis();
    clearLCD = true;
  }
}

void displayInfo(String topText, String bottomText)
{  
  lcd.setCursor(0,0);
  lcd.print(padString(topText));
//  lcd.print(topText);
  lcd.setCursor(0,1);
//  lcd.print(bottomText);
  lcd.print(padString(bottomText));
}

String padString(String value)
{
  char padding[value.length()];
  for(int count = 0; count < sizeof(padding); ++count)
    padding[count] = ' ';
  return String(value) + String(padding);
}

String floatToString(float value)
{
  char result[5];
  dtostrf(value, 3, 1, result);
  return result;
}
