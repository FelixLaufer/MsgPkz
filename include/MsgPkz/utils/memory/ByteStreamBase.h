#ifndef _BYTE_STREAM_BASE_H_
#define _BYTE_STREAM_BASE_H_

#include "../arx/XPlattform.h"

class ByteStreamBase
{
public:
  ByteStreamBase()
    : dataStartPtr_(nullptr)
    , dataEndPtr_(nullptr)
  {}

  ByteStreamBase(uint8_t* const data, const size_t size)
    : dataStartPtr_(data)
    , dataEndPtr_(data + size)
  {}

  uint8_t* const data() const
  {
    return dataStartPtr_;
  };

  size_t size() const
  {
    assert(dataEndPtr_ >= dataStartPtr_);
    return dataEndPtr_ - dataStartPtr_;
  };

  void clear()
  {
    memset(dataStartPtr_, 0, size());
  }

protected:
  uint8_t* const dataStartPtr_;
  uint8_t* const dataEndPtr_;
};

#endif