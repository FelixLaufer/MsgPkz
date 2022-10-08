#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "TCPClient.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <map>
#include <mutex>
#include <string>

class TCPServer : public Stream<boost::asio::ip::tcp::socket>
{
public:
  class TCPEndpoint : public StreamShared<boost::asio::ip::tcp::socket>
  {
  public:
    TCPEndpoint(boost::asio::ip::tcp::socket&& socket, boost::asio::io_context& io)
      : StreamShared<boost::asio::ip::tcp::socket>(std::move(socket), io)
    {
      start();
      endpoint_ = stream_->remote_endpoint();
      address_ = endpoint_.address().to_string() + ":" + std::to_string(endpoint_.port());
    }

    virtual ~TCPEndpoint()
    {
      stop();
    }

    using StreamShared<boost::asio::ip::tcp::socket>::onError;
    using StreamShared<boost::asio::ip::tcp::socket>::writeWait;
    using StreamShared<boost::asio::ip::tcp::socket>::writeAsync;
    using StreamShared<boost::asio::ip::tcp::socket>::readUntilWait;
    using StreamShared<boost::asio::ip::tcp::socket>::readUntilAsync;

    const boost::asio::ip::tcp::endpoint& getEndpoint() const
    {
      return endpoint_;
    }

    const std::string& getAddress() const
    {
      return address_;
    }

    protected:
      boost::asio::ip::tcp::endpoint endpoint_;
      std::string address_;
  };

  TCPServer(const uint16_t port = 22)
    : Stream<boost::asio::ip::tcp::socket>()
    , acceptor_(io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , mtx_()
    , acceptedCallback_([](const std::string&) {})
    , lostCallback_([](const std::string&) {})
  {
    accept();
    start();
  }

  ~TCPServer()
  {
    stop();
  }

  bool hasClients()
  {
    mtx_.lock();
    const bool ret = !clients_.empty();
    mtx_.unlock();
    return ret;
  }

  size_t numClients()
  {
    mtx_.lock();
    const size_t ret = clients_.size();
    mtx_.unlock();
    return ret;
  }

  bool hasClient(const std::string& client)
  {
    mtx_.lock();
    const bool ret = clients_.count(client);
    mtx_.unlock();
    return ret;
  }

  std::vector<std::string> getClients()
  {
    std::vector<std::string> ret;
    mtx_.lock();
    for (const auto& kv : clients_)
      ret.emplace_back(kv.first);
    mtx_.unlock();
    return ret;
  }

  void waitForClients(const size_t numClients = 1)
  {
    while (clients_.size() < numClients);
  }

  void onClientConnected(std::function<void(const std::string&)> onClientAccepted)
  {
    acceptedCallback_ = std::move(onClientAccepted);
  }

  void onClientDisconnected(std::function<void(const std::string&)> onClientLost)
  {
    lostCallback_ = std::move(onClientLost);
  }

  void sendToWait(const std::string& client, const ByteStream& bs)
  {
    forClient(client, [&bs](const std::shared_ptr<TCPEndpoint> c) { c->writeWait(bs); });
  }

  void sendToAsync(const std::string& client, const ByteStream& bs)
  {
    forClient(client, [&bs](const std::shared_ptr<TCPEndpoint> c) { c->writeAsync(bs); });
  }

  void sendWait(const ByteStream& bs)
  {
    foreachClient([&bs](const std::shared_ptr<TCPEndpoint> c) { c->writeWait(bs); });
  }

  void sendAsync(const ByteStream& bs)
  {
    foreachClient([&bs](const std::shared_ptr<TCPEndpoint> c) { c->writeAsync(bs); });
  }

  template <typename TReadCallback>
  void receiveFromWait(const std::string& client, TReadCallback&& readCallback, const uint8_t delimiter)
  {
    forClient(client, [&readCallback, delimiter](const std::shared_ptr<TCPEndpoint> c)
    {
      c->readUntilWait(std::forward<TReadCallback>(readCallback), delimiter, false);
    });
  }

  template <typename TKeepAlive, typename TReadCallback>
  void receiveFromAsync(const std::string& client, TKeepAlive&& keepAlive, TReadCallback&& readCallback, const uint8_t delimiter)
  {
    forClient(client, [&, delimiter](const std::shared_ptr<TCPEndpoint> c)
    {
      c->readUntilAsync(std::forward<TKeepAlive>(keepAlive), std::forward<TReadCallback>(readCallback), delimiter, false);
    });
  }

  template <typename TReadCallback>
  void receiveWait(TReadCallback&& readCallback, const uint8_t delimiter)
  {
    foreachClient([&, delimiter](const std::shared_ptr<TCPEndpoint> c)
    {
      c->readUntilWait(std::forward<TReadCallback>(readCallback), delimiter, false);
    });
  }

  template <typename TReadCallback>
  void receiveAsync(TReadCallback&& readCallback, const uint8_t delimiter)
  {
    foreachClient([&, delimiter](const std::shared_ptr<TCPEndpoint> c)
    {
      c->readUntilAsync(std::forward<TReadCallback>(readCallback), delimiter, false);
    });
  }

  template <typename TReadCallback>
  void subscribeTo(const std::string& client, TReadCallback&& readCallback, const uint8_t delimiter)
  {
    forClient(client, [&, delimiter](const std::shared_ptr<TCPEndpoint> c)
    {
      c->readUntilAsync(std::forward<TReadCallback>(readCallback), delimiter, true);
    });
  }

  template <typename TReadCallback>
  void subscribe(TReadCallback&& readCallback, const uint8_t delimiter)
  {
    foreachClient([&, delimiter](const std::shared_ptr<TCPEndpoint> c)
    {
      c->readUntilAsync(*this, std::forward<TReadCallback>(readCallback), delimiter, true);
    });
  }

protected:
  template <typename TFunction>
  void forClient(const std::string& name, TFunction&& func)
  {
    mtx_.lock();
    if (clients_.count(name))
    {
      if (clients_.at(name))
        func(clients_.at(name));
      else
        clients_.erase(name);
    }  
    mtx_.unlock();
  }

  template <typename TFunction>
  void foreachClient(TFunction&& func)
  {
    mtx_.lock();
    for (auto it = clients_.begin(); it != clients_.end();)
    {
      if (it->second)
        func(it++->second);
      else
        it = clients_.erase(it);
    }
    mtx_.unlock();
  }

private:
  void accept()
  {
    stream_.emplace(io_);
    acceptor_.async_accept(*stream_, [&](boost::system::error_code error)
      {
        if (error)
          std::cerr << "TCP Server: unable to accept new client." << std::endl;
        else
        {
          stream_->set_option(boost::asio::ip::tcp::no_delay(true));
          stream_->set_option(boost::asio::socket_base::send_buffer_size(65536));
          stream_->set_option(boost::asio::socket_base::receive_buffer_size(65536));
          const auto client = std::make_shared<TCPEndpoint>(std::move(*stream_), io_);
          mtx_.lock();
          clients_.insert({ client->getAddress(), client });
          mtx_.unlock();
		      const auto weak = std::weak_ptr<TCPEndpoint>(client);
          client->onError(
            [&, weak]
            {         
              const auto shared = weak.lock();
              if (shared)
              {
                try
                {
                  const std::string clientAddress = shared->getAddress();
                  mtx_.lock();
                  if (clients_.count(clientAddress))
                  {
                    clients_.erase(clientAddress);
                    lostCallback_(clientAddress); 
                  }
                  mtx_.unlock();
                }
                catch(...)
                {}
              }
            });
          if (acceptedCallback_)
            acceptedCallback_(client->getAddress());
        }
        accept();
      });
  }

  boost::asio::ip::tcp::acceptor acceptor_;
  std::mutex mtx_;
  std::map<std::string, std::shared_ptr<TCPEndpoint>> clients_;
  std::function<void(const std::string&)> acceptedCallback_;
  std::function<void(const std::string&)> lostCallback_;
};

#endif
