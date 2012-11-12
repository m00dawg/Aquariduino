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
    /* If the heater was on, we know we have completed a 
     * cycle, so let's count it */
    if(heater)
      ++heaterCycles;
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

