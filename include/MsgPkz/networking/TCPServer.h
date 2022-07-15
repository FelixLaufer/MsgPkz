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

    boost::asio::ip::tcp::endpoint getEndpoint() const
    {
      return stream_->remote_endpoint();
    }

    std::string getName() const
    {
      return getEndpoint().address().to_string() + ":" + std::to_string(getEndpoint().port());
    }
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

  void onClientLost(std::function<void(const std::string&)> onClientLost)
  {
    lostCallback_ = std::move(onClientLost);
  }

protected:
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
          clients_.insert({ client->getName(), client });
          mtx_.unlock();
		      auto weak = std::weak_ptr<TCPEndpoint>(client);
          client->onError(
            [&, weak]
            {          
              const auto shared = weak.lock();
              if (shared)
              {
                try {
                  const std::string clientName = shared->getName();
                  clients_.at(clientName) = nullptr;
                  lostCallback_(clientName);
                }
                catch(...)
                { }
              }
            });
          if (acceptedCallback_)
            acceptedCallback_(client->getName());
        }
        accept();
      });
  }

  boost::asio::ip::tcp::acceptor acceptor_;
  std::mutex mtx_;
  std::map<std::string, std::shared_ptr<TCPEndpoint>> clients_;
  std::vector<std::string> invalidClients_;
  std::function<void(const std::string&)> acceptedCallback_;
  std::function<void(const std::string&)> lostCallback_;
};

#endif