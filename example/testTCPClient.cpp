#include <MsgPkz/MsgPkzTCPClient.h>

int main(int argc, char** argv)
{
  MsgPkzTCPClient client("127.0.0.1", 8198);

  client.subscribe(
    [](const FrameMessage& fm, const IMUMessage& im)
    {
      std::cout << "IMU message:"
        << " ts: " << im.timestamp
        << " acc: " << im.acc.vec.x << "," << im.acc.vec.y << "," << im.acc.vec.z
        << " gyr: " << im.gyr.vec.x << "," << im.gyr.vec.y << "," << im.gyr.vec.z
        << " mag: " << im.mag.vec.x << "," << im.mag.vec.y << "," << im.mag.vec.z
        << std::endl;
    },
    [](const FrameMessage& fm, const IMUMessage& im0, const IMUMessage& im1, const IMUMessage& im2)
    {
      std::cout << "I received a packet with exactly 3 IMU messages!" << std::endl;
    },
    [&](const StatusMessage& sm)
    {
      std::cout << "Status update: " << sm.id << std::endl;
      client.sendAsync(StatusMessage(0x01)); // Send something back
    },
    [](const ErrorMessage& em)
    {
      std::cerr << "Oops... something went wrong!" << std::endl;
    }
  );

  while (true)
  {
    // Meanwhile in the main thread ...
  }

  return 0;
}