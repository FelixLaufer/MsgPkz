#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "StreamBase.h"

class Serial : public Stream<boost::asio::serial_port>
{
public:
  Serial(const std::string device, const uint32_t baudRate)
    : Stream()
  {
    try
    {
      stream_->open(device);
      stream_->set_option(boost::asio::serial_port_base::baud_rate(baudRate));
      stream_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    }
    catch (...)
    {
      std::cerr << "Serial: unable to open serial port (" << device << ")." << std::endl;
    }
    start();
  }

  ~Serial()
  {
    stop();
  }
};

#endif