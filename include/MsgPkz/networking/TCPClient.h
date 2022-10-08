#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "StreamBase.h"

#include <boost/asio.hpp>

class TCPClient : public Stream<boost::asio::ip::tcp::socket>
{
public:
  TCPClient() = delete;

  TCPClient(const std::string& address, const uint16_t port)
    : Stream<boost::asio::ip::tcp::socket>()
  {
    try
    {
      stream_->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address_v4(address), port));
      stream_->set_option(boost::asio::ip::tcp::no_delay(true));
      stream_->set_option(boost::asio::socket_base::send_buffer_size(65536));
      stream_->set_option(boost::asio::socket_base::receive_buffer_size(65536));
    }
    catch (...)
    {
      std::cerr << "TCPClient: unable to connect to server (" << address << ":" << port << ")." << std::endl;
      return;
    }
    
    start();
  }

  virtual ~TCPClient()
  {
    stop();
  }

  void sendWait(ByteStream& bs)
  {
    writeWait(bs);
  }

  void sendAsync(ByteStream& bs)
  {
    writeAsync(bs);
  }

  template <typename TReadCallback>
  void receiveWait(TReadCallback&& readCallback, const uint8_t delimiter)
  {
    readUntilWait(readCallback, delimiter, false);
  }

  template <typename TReadCallback>
  void receiveAsync(TReadCallback&& readCallback, const uint8_t delimiter)
  {
    readUntilAsync(std::forward<TReadCallback>(readCallback), delimiter, false);
  }

 template <typename TReadCallback>
  void subscribe(TReadCallback&& readCallback, const uint8_t delimiter)
  {
    readUntilAsync(std::forward<TReadCallback>(readCallback), delimiter, true);
  }
};

#endif
