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


//Convert given times to secnds, then compare with
//elapsedSecsToday() - Number of seconds since midnight

//void followSchedule(

/*
 * Function to control devices on a schedule
 */
void deviceOn(DeviceOnSchedule device)
{
  digitalWrite(device.pin, HIGH);
  device.state = true; 
}

void deviceOff(DeviceOnSchedule device)
{
    digitalWrite(device.pin, LOW);
    device.state = false; 
}
