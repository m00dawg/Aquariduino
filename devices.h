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

void checkSchedule(DeviceOnSchedule &device)
{
  unsigned long startSeconds = device.schedule[0] * (long)3600 + device.schedule[1] * 60;
  unsigned long stopSeconds = device.schedule[2] * (long)3600 + device.schedule[3] * 60;
  unsigned long currentSeconds = elapsedSecsToday(now());
  /*
  Serial.print("Stop/Start/Current: ");
  Serial.print((String)stopSeconds);
  Serial.print(" ");
  Serial.print((String)startSeconds);
  Serial.print(" ");
  Serial.println((String)currentSeconds);
  */
  if(currentSeconds > startSeconds & stopSeconds > currentSeconds)
  {
    //Serial.println("Device On");
    digitalWrite(device.pin, HIGH);
    device.state = true;
  }
  else
  {
    //Serial.println("Device Off");
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


