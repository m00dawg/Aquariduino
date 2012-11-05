/*
 * Structure to handle any device on a schedule
 * That is a device that is on or off depending on the time
 *
 * TODO: Overrides for manual control where it won't mess up
 * the schedule
 */
struct DeviceOnSchedule{
  int schedule[4];
  int pin;
  boolean state;
};

/*
 * Function to control devices on a schedule
 */
void controlDeviceOnSchedule(DeviceOnSchedule device)
{
  
  if(hour(now()) >= device.schedule[0] & 
    minute(now()) >= device.schedule[1] &
    hour(now()) <= device.schedule[2] & 
    minute(now()) <= device.schedule[3])
  {
    digitalWrite(device.pin, HIGH);
    device.state = true; 
  }
  else
  {
    digitalWrite(device.pin, LOW);
    device.state = false; 
  }
}

