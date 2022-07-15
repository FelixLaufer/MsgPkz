#ifdef ARDUINO
// Note for Arduino Micro (Atmega32u4):
// Serial is the USB serial. Use Serial1 instead in order to use the physical hardware UART (TX, RX pins) and also replace Serial with Serial1 in ExoClient.h.

#include <MsgPkz/MsgPkzSerialClient.h>

MsgPkzSerialClient client;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(1000000); // 1 MBaud
  while(!Serial); // Wait until Serial is ready
}

void loop()
{  
  client.receive
  (
    [](const FrameMessage& fm, const IMUMessage& im0, const IMUMessage& im1, const IMUMessage& im2)
    {
      digitalWrite(LED_BUILTIN, HIGH);
    },
    [](const FrameMessage& fm, const IMUMessage& im)
    {
      digitalWrite(LED_BUILTIN, LOW);
    }
  );
}


#endif