#ifndef _MSG_PKZ_TCP_SERVER_H_
#define _MSG_PKZ_TCP_SERVER_H_

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
  using TCPServer::onClientLost;

  template <typename... TMessages>
  void sendToWait(const std::string& client, TMessages&&... messages)
  {
    mtx_.lock();
    if (clients_.count(client))
      clients_.at(client)->writeWait(packetize(std::forward<TMessages>(messages)...));
    mtx_.unlock();
  }

  template <typename... TMessages>
  void sendToAsync(const std::string& client, TMessages&&... messages)
  {
    mtx_.lock();
    if (clients_.count(client))
      clients_.at(client)->writeAsync(packetize(std::forward<TMessages>(messages)...));
    mtx_.unlock();
  }

  template <typename... TMessages>
  void sendWait(TMessages&&... messages)
  {
    mtx_.lock();
    auto it = clients_.begin();
    while(it != clients_.end())
    {
      if (it->second)
        it++->second->writeWait(packetize(std::forward<TMessages>(messages)...));
      else
        it = clients_.erase(it);
    }
    mtx_.unlock();
  }

  template <typename... TMessages>
  void sendAsync(TMessages&&... messages)
  {
    mtx_.lock();
    auto it = clients_.begin();
    while (it != clients_.end())
    {
      if (it->second)
        it++->second->writeAsync(packetize(std::forward<TMessages>(messages)...));
      else
        it = clients_.erase(it);
    }
    mtx_.unlock();
  }

  template <typename... TMessageCallbacks>
  void receiveFromWait(const std::string& client, TMessageCallbacks&&... messageCallbacks)
  {
    mtx_.lock();
    if (clients_.count(client))
      invokeWait(clients_.at(client), false, std::forward<TMessageCallbacks>(messageCallbacks)...);
    mtx_.unlock();
  }

  template <typename... TMessageCallbacks>
  void receiveFromAsync(const std::string& client, TMessageCallbacks&&... messageCallbacks)
  {
    mtx_.lock();
    if (clients_.count(client))
      invokeAsync(clients_.at(client), false, std::forward<TMessageCallbacks>(messageCallbacks)...);
    mtx_.unlock();
  }

  template <typename... TMessageCallbacks>
  void receiveWait(TMessageCallbacks&&... messageCallbacks)
  {
    mtx_.lock();
    auto it = clients_.begin();
    while (it != clients_.end())
    {
      if (it->second)
        invokeWait(it++->second, false, std::forward<TMessageCallbacks>(messageCallbacks)...);
      else
        it = clients_.erase(it);
    }
    mtx_.unlock();
  }

  template <typename... TMessageCallbacks>
  void receiveAsync(TMessageCallbacks&&... messageCallbacks)
  {
    mtx_.lock();
    auto it = clients_.begin();
    while (it != clients_.end())
    {
      if (it->second)
        invokeAsync(it++->second, false, std::forward<TMessageCallbacks>(messageCallbacks)...);
      else
        it = clients_.erase(it);
    }
    mtx_.unlock();
  }

  template <typename... TMessageCallbacks>
  void subscribeTo(const std::string& client, TMessageCallbacks&&... messageCallbacks)
  {
    mtx_.lock();
    if (clients_.count(client))
      invokeAsync(clients_.at(client), true, std::forward<TMessageCallbacks>(messageCallbacks)...);
    mtx_.unlock();
  }

  template <typename... TMessageCallbacks>
  void subscribe(TMessageCallbacks&&... messageCallbacks)
  {
    mtx_.lock();
    auto it = clients_.begin();
    while (it != clients_.end())
    {
      if (it->second)
        invokeAsync(it++->second, true, std::forward<TMessageCallbacks>(messageCallbacks)...);
      else
        it = clients_.erase(it);
    }
    mtx_.unlock();
  }

private:
  template <typename... TMessageCallbacks>
  void invokeWait(std::shared_ptr<TCPEndpoint> client, bool repeat, TMessageCallbacks&&... messageCallbacks)
  {
    client->readUntilWait
    (
      [&](ByteStream& ps)
      {
        depacketize(ps, std::forward<TMessageCallbacks>(messageCallbacks)...);
      }
      , PACKET_DELIMITER, true
    );
  }

  template <typename... TMessageCallbacks>
  void invokeAsync(std::shared_ptr<TCPEndpoint> client, bool repeat, TMessageCallbacks&&... messageCallbacks)
  {
    client->readUntilAsync
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