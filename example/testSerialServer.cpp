#include <MsgPkz/MsgPkzSerialServer.h>

#include <chrono>
#include <thread>

int main(int argc, char** argv)
{
  std::string device;
  #ifdef WIN32
    device = "COM1";
  #else
    device = "/dev/ttyUSB0";
  #endif

  MsgPkzSerialServer server(device);

  uint32_t ts = 0;
  while (true)
  {
    if (ts % 100 == 0)
      server.sendAsync(FrameMessage(ts), IMUMessage(0, ts, Vec3(0, 0, 9.81), Vec3(0, 0, 0), Vec3(1, 0, 0)));
    else
      server.sendAsync(FrameMessage(), IMUMessage(), IMUMessage(), IMUMessage());

    if (ts % 1000 == 0)
      std::cout << "A server must do what a server must do..." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ts += 10;
  }

  return 0;
}