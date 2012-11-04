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
  "H:" + (String)heater + " L:" + (String)light.state);
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

