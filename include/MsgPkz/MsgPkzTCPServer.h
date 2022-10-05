#ifndef ARDUINO

#ifndef _MSG_PKZ_TCP_SERVER_H_
#define _MSG_PKZC_TCP_SERVER_H_

#include "packetizing/Packetizer.h"
#include "networking/TCPServer.h"

class MsgPkzTCPServer : protected TCPServer, protected Packetizer
{
public:
  MsgPkzTCPServer(const uint16_t port)
    : TCPServer(port)
    , Packetizer()
  {}

  ~MsgPkzTCPServer() = default;

  using TCPServer::numClients;
  using TCPServer::hasClients;
  using TCPServer::getClients;
  using TCPServer::waitForClients;
  using TCPServer::onClientConnected;
  using TCPServer::onClientDisconnected;

  template <typename... TMessages>
  void sendToWait(const std::string& client, TMessages&&... messages)
  {
    TCPServer::sendToWait(client, packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessages>
  void sendToAsync(const std::string& client, TMessages&&... messages)
  {
    TCPServer::sendToAsync(client, packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessages>
  void sendWait(TMessages&&... messages)
  {
    TCPServer::sendWait(packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessages>
  void sendAsync(TMessages&&... messages)
  {
    TCPServer::sendAsync(packetize(std::forward<TMessages>(messages)...));
  }

  template <typename... TMessageCallbacks>
  void receiveFromWait(const std::string& client, TMessageCallbacks&&... messageCallbacks)
  {
    TCPServer::receiveFromWait(client, [&](ByteStream& ps)
    {
      depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }

  template <typename... TMessageCallbacks>
  void receiveFromAsync(const std::string& client, TMessageCallbacks&&... messageCallbacks)
  {
    TCPServer::receiveFromAsync(client, *this, [&](decltype(*this)&& self, ByteStream& ps)
    {
      self.depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }

  template <typename... TMessageCallbacks>
  void receiveWait(TMessageCallbacks&&... messageCallbacks)
  {
    TCPServer::receiveWait([&](ByteStream& ps)
    {
      depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }

  template <typename... TMessageCallbacks>
  void receiveAsync(TMessageCallbacks&&... messageCallbacks)
  {
    TCPServer::receiveAsync(*this, [&](decltype(*this)&& self, ByteStream& ps)
    {
      self.depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }

  template <typename... TMessageCallbacks>
  void subscribeTo(const std::string& client, TMessageCallbacks&&... messageCallbacks)
  {
    TCPServer::subscribeTo(client, *this, [&](decltype(*this)&& self, ByteStream& ps)
    {
      self.depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }

  template <typename... TMessageCallbacks>
  void subscribe(TMessageCallbacks&&... messageCallbacks)
  {
    TCPServer::subscribe(*this, [&](decltype(*this)&& self, ByteStream& ps)
    {
      self.depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
    }, PACKET_DELIMITER);
  }
};

#endif

#endif
