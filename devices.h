/*
 * -------
 * STRUCTS
 * -------
 */
struct DeviceOnSchedule{
  int schedule[4];
  int pin;
  boolean state;
};

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

