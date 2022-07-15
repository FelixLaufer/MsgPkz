#ifndef _MSG_PKZ_SERIAL_SERVER_H_
#define _MSG_PKZ_SERIAL_SERVER_H_

#include "packetizing/Packetizer.h"
#include "networking/Serial.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <string>
#include <iostream>

class MsgPkzSerialServer : public Serial, protected Packetizer
{
public:
  MsgPkzSerialServer(const std::string device, const uint32_t baudRate = 1000000)
    : Serial(device, baudRate)
    , Packetizer()
  {}

  ~MsgPkzSerialServer() = default;

  template <typename... TMessages>
  void sendWait(TMessages&&... messages)
  {
    writeWait(packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessages>
  void sendAsync(TMessages&&... messages)
  {
    writeAsync(packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessageCallbacks>
  void receiveWait(TMessageCallbacks&&... messageCallbacks)
  {
    readUntilWait
    (
      [&](ByteStream& ps)
      {
        depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
      }
      , PACKET_DELIMITER, false
    );
  }

  template <typename... TMessageCallbacks>
  void receiveAsync(TMessageCallbacks&&... messageCallbacks)
  {
    readUntilAsync
    (
      *this,
      [&](decltype(*this) && self, ByteStream& ps)
      {
        self.depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
      }
      , PACKET_DELIMITER, false
    );
  }

  template <typename... TMessageCallbacks>
  void subscribe(TMessageCallbacks&&... messageCallbacks)
  {
    readUntilAsync
    (
      *this,
      [&](decltype(*this) && self, ByteStream& ps)
      {
        self.depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
      }
      , PACKET_DELIMITER, true
    );
  }
};

#endif