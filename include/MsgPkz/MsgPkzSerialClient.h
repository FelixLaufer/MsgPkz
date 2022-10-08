#ifdef ARDUINO

#ifndef _MSG_PKZ_SERIAL_CLIENT_H_
#define _MSG_PKZ_SERIAL_CLIENT_H_

#include "packetizing/Packetizer.h"

class MsgPkzSerialClient
{
public:
  MsgPkzSerialClient()
    : recvS_(ByteStream(recBuf_.data(), recBuf_.size()))
    , packetizer_()
  {}
  
  ~MsgPkzSerialClient() = default;

  template <typename... TMessages>
  void send(TMessages&&... messages)
  {
    write(packetizer_.packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessageCallbacks>
  void receive(TMessageCallbacks&&... messageCallbacks)
  {
    read([&](ByteStream& ps) { packetizer_.depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...); });
  }

protected:
  void write(const ByteStream& bs)
  {
    Serial.write(bs.data(), bs.size());
  }

  template <typename TReceiveCallback>
  void read(TReceiveCallback&& receiveCallback)
  {
    const bool delimiterFound = readAndCheckForDelimiter();
    if (delimiterFound)
    {
      ByteStream ps(recvS_.data(), recvS_.tellp());
      receiveCallback(ps);
      recvS_.resetw();
    }
  }

  bool readAndCheckForDelimiter()
  {
    const int c = Serial.read();
    if (c < 0)
      return false;

    assert(bs.availablep());
    recvS_.put(static_cast<uint8_t>(c));

    if (static_cast<uint8_t>(c) == Packetizer::PACKET_DELIMITER)
      return true;

    return false;
  }

  Buffer<Packetizer::MAX_MESSAGE_SIZE * Packetizer::MAX_MESSAGES> recBuf_;
  ByteOutStream recvS_;
  Packetizer packetizer_;
};

#endif

#endif
