unsigned long getUnixTimeFromNTP()
{
  const unsigned long seventy_years = 2208988800UL;
  unsigned long highWord, lowWord, epoch;

  memset(ntpBuffer, 0, ntpPacketSize);
  udp.begin(ntpLocalPort);
  sendNTPPacket(ntpServer, ntpPort);
  delay(msInSecond);

  if(udp.parsePacket())
  {
    udp.read(ntpBuffer, ntpPacketSize);
    highWord = word(ntpBuffer[40], ntpBuffer[41]);
    lowWord = word(ntpBuffer[42], ntpBuffer[43]);  
    epoch = highWord << 16 | lowWord;
    epoch = epoch - seventy_years;
    return epoch;
  }
  return 0; // return 0 if unable to get the time
}

unsigned long sendNTPPacket(byte *address, uint16_t port)
{
  // set all bytes in the buffer to 0
  memset(ntpBuffer, 0, ntpPacketSize);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  ntpBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntpBuffer[1] = 0;     // Stratum, or type of clock
  ntpBuffer[2] = 6;     // Polling Interval
  ntpBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntpBuffer[12]  = 49;
  ntpBuffer[13]  = 0x4E;
  ntpBuffer[14]  = 49;
  ntpBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  udp.beginPacket(address, port); //NTP requests are to port 123
  udp.write(ntpBuffer,ntpPacketSize);
  udp.endPacket(); 
}

String printTime(time_t time)
{
  return (String)hour(time) + ":" + (String)minute(time) + ":" + (String)second(time);
}


