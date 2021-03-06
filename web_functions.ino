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

void webPrintRawStats()
{
  if(multiLineOutput)
    webClient.println("Temp:" + String(floatToString(currentTemp)) + "\nHeater:" + heater + "\nLight:" + light.state + "\n");
  else
    webClient.println("Temp:" + String(floatToString(currentTemp)) + " Heater:" + heater + " Light:" + light.state);
}
