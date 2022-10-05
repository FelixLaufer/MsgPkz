#ifndef _STREAM_BASE_H_
#define _STREAM_BASE_H_

#include "../utils/memory/ByteStream.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <functional>
#include <thread>

template <typename TStream, bool IsShared = false>
class StreamBase : public std::enable_shared_from_this<StreamBase<TStream, IsShared>>
{
protected:
  static constexpr bool VERBOSE = false;

  StreamBase()
    : stream_()
    , readStrand_()
    , writeStrand_()
    , recvBuf_()
    , errorCallback_([]() {})
  {}

  virtual ~StreamBase() {}

  template<typename R = std::shared_ptr<StreamBase<TStream, IsShared>>>
  typename std::enable_if<!IsShared, R>::type getSharedFromThisOrNull()
  {
    return R{};
  }

  template<typename R = std::shared_ptr<StreamBase<TStream, IsShared>>>
  typename std::enable_if<IsShared, R>::type getSharedFromThisOrNull()
  {
    return this->shared_from_this();
  }

  virtual void start() = 0;

  virtual void stop() = 0;

  void onError(std::function<void()>&& errorCallback)
  {
    errorCallback_ = std::move(errorCallback);
  }

  void writeWait(const ByteStream& bs)
  {
    try
    {
      boost::asio::write(*stream_, boost::asio::buffer(bs.data(), bs.size()), boost::asio::transfer_all());
    }
    catch (...)
    {
      if (VERBOSE)
        std::cerr << "StreamBase: unable to write (blocking)." << std::endl;
      errorCallback_();
    }
  }

  void writeAsync(const ByteStream& bs)
  {
    try
    {
      auto self = getSharedFromThisOrNull();
      const auto size = bs.size();
      boost::asio::async_write(*stream_, boost::asio::buffer(bs.data(), bs.size()), boost::asio::transfer_all(),
        boost::asio::bind_executor(*writeStrand_,
          [&, this, self, size](const boost::system::error_code error, size_t bytesTransferred)
          {
            if (error || bytesTransferred < size)
            {
              if (VERBOSE)
                std::cerr << "StreamBase: unable to write (async)." << std::endl;
              errorCallback_();
            }
          }));
    }
    catch(...)
    {
      if (VERBOSE)
        std::cerr << "StreamBase: unable to write (async)." << std::endl;
      errorCallback_();
    }
  }

  template <typename TReadCallback>
  void readUntilWait(TReadCallback&& readCallback, const uint8_t delimiter, const bool repeat)
  {
    try
    {
      const size_t bytesTransferred = read_until(*stream_, recvBuf_, delimiter);
      ByteStream bs(const_cast<uint8_t*>(static_cast<const uint8_t*>(recvBuf_.data().data())), bytesTransferred);
      readCallback(bs);
    }
    catch (...)
    {
      if (VERBOSE)
        std::cerr << "StreamBase: unable to read (blocking)." << std::endl;
      errorCallback_();
    }
  }

  template <typename TKeepAlive, typename TReadCallback>
  void readUntilAsync(TKeepAlive&& keepAlive, TReadCallback&& readCallback, const uint8_t delimiter, const bool repeat)
  {
    try
    {
      auto self = getSharedFromThisOrNull();
      boost::asio::async_read_until(*stream_, recvBuf_, delimiter,
        boost::asio::bind_executor(*readStrand_,
          [&, this, self, delimiter, repeat](const boost::system::error_code error, size_t bytesTransferred)
          {
            if (error)
            {
              if (VERBOSE)
                std::cerr << "StreamBase: unable to read (async)." << std::endl;
              errorCallback_();
            }
            else
            {
              ByteStream bs(const_cast<uint8_t*>(static_cast<const uint8_t*>(recvBuf_.data().data())), bytesTransferred);
              readCallback(keepAlive, bs);
              recvBuf_.consume(bytesTransferred);
              if (repeat)
                readUntilAsync(std::forward<TKeepAlive>(keepAlive), std::forward<TReadCallback>(readCallback), delimiter, repeat);
            }
          }));
    }
    catch (...)
    {
      if (VERBOSE)
        std::cerr << "StreamBase: unable to read (async)." << std::endl;
      errorCallback_();
    }
  }

  boost::optional<TStream> stream_;
  boost::optional<boost::asio::io_service::strand> readStrand_;
  boost::optional<boost::asio::io_service::strand> writeStrand_;
  boost::asio::streambuf recvBuf_;
  std::function<void()> errorCallback_;
};

template <typename TStream>
class Stream : public StreamBase<TStream, false>
{
protected:
  Stream()
    : StreamBase<TStream, false>()
    , io_()
    , work_(io_)
    , thread_()
  {
    this->stream_.emplace(io_);
    this->readStrand_.emplace(io_);
    this->writeStrand_.emplace(io_);
  }

  virtual ~Stream()
  {
    stop();
  }

  void start()
  {
    thread_ = std::thread(boost::bind(&boost::asio::io_context::run, &io_));
  }

  void stop()
  {
    try
    {
      this->stream_->close();
      io_.stop();
      if (thread_.joinable())
        thread_.join();
    }
    catch (...) {}
  }

  boost::asio::io_context io_;
  boost::asio::io_context::work work_;
  std::thread thread_;
};

template <typename TStream>
class StreamShared : public StreamBase<TStream, true>
{
protected:
  StreamShared() = delete;

  StreamShared(TStream&& stream, boost::asio::io_context& io)
    : StreamBase<TStream, true>()
  {
    this->stream_.emplace(std::move(stream));
    this->readStrand_.emplace(io);
    this->writeStrand_.emplace(io);
  }

  virtual ~StreamShared()
  {
    stop();
  }

  void start()
  {}

  void stop()
  {
    try
    {
      this->stream_->close();
    }
    catch (...) {}
  }
};

#endif
