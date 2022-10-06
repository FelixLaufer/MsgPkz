# MsgPkz - Lightweight Message Packetizer
This is a lightweight header-only messaging library with a core implementation that does not rely on any dynamic memory.
It is suitable for systems with very limited ressources such as microcontrollers.
MsgPkz originated from a project where an embedded application processor (Qualcomm® RB5) had to communicate wirelessly with an external server via TCP and with a tiny microcontroller via UART.
For this reason, there are TCP and serial endpoints available as well as a serial client implementation for Arduino-like boards. Thanks to [this guy](https://github.com/hideakitai) for the amazing Arduino port of [STL-like containers](https://github.com/hideakitai/ArxContainer) and [type traits](https://github.com/hideakitai/ArxTypeTraits).

Messages are identified and depacketized with an unique 1-byte id, packets can consist of any number and kind of messages (adapt the maximum buffer sizes, if necessary).
You can subscribe to specific packets by explicitely describing their message signature via lambda functions. The code to handle the packets with types of messages that you are interested in is actually generated at compile time. There is some happy meta template programming magic going on in the background... :upside_down_face:
Feel free to customize the message types to your needs. Just make sure that the size property sums up to the number of bytes of your payload.

## Features
- super fast message packetizer and de-packetizer
- easily customizable and extendable
- Arduino compatible
- core without dynamic memory allocation
- packet framing with either [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) or [SLIP](https://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol)
- CRC16-CCITT secured packets
- pre-implemented TCP and serial endpoints (Boost.Asio and Arduino Serial)
- subscribe, receive (async/blocking) and send (async/blocking)

## Usage
```cpp
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
```

## Requires
- Boost::system ≥ 1.70

