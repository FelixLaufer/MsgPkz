#ifndef ARDUINO

#ifndef _MSG_PKZ_TCP_CLIENT_H_
#define _MSG_PKZ_TCP_CLIENT_H_

#include "packetizing/Packetizer.h"
#include "networking/TCPClient.h"

class MsgPkzTCPClient : protected TCPClient, protected Packetizer
{
public:
  MsgPkzTCPClient(const std::string& address, const uint16_t port)
    : TCPClient(address, port)
    , Packetizer()
  {}

  ~MsgPkzTCPClient() = default;

  template <typename... TMessages>
  void sendWait(TMessages&&... messages)
  {
    TCPClient::sendWait(packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessages>
  void sendAsync(TMessages&&... messages)
  {
    TCPClient::sendAsync(std::forward<TMessages>(messages)...);
  }

  template <typename... TMessageCallbacks>
  void receiveWait(TMessageCallbacks&&... messageCallbacks)
  {
    TCPClient::receiveWait([&](ByteStream& ps)
    {
      depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }

  template <typename... TMessageCallbacks>
  void receiveAsync(TMessageCallbacks&&... messageCallbacks)
  {
    TCPClient::receiveAsync([&](ByteStream& ps)
    {
      depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }

  template <typename... TMessageCallbacks>
  void subscribe(TMessageCallbacks&&... messageCallbacks)
  {
    TCPClient::subscribe([&](ByteStream& ps)
    {
      depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }
};

#endif

#endif
