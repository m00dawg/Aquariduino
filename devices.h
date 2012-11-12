#include <Time.h>

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

void checkSchedule(DeviceOnSchedule device)
{
  int startSeconds = device.schedule[0] * 3600 + device.schedule[1] * 60;
  int stopSeconds = device.schedule[1] * 3600 + device.schedule[2] * 60;
  int currentSeconds = elapsedSecsToday(now());
  if(currentSeconds > startSeconds & stopSeconds > currentSeconds)
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


