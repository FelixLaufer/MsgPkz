#ifndef _BYTE_OUT_STREAM_H_
#define _BYTE_OUT_STREAM_H_

#include "ByteStreamBase.h"

class ByteOutStream : virtual public ByteStreamBase
{
public:
  ByteOutStream()
    : ByteStreamBase()
    , writePtr_(nullptr)
  {}

  ByteOutStream(const ByteStreamBase& bs)
    : ByteStreamBase(bs)
    , writePtr_(bs.data())
  {}

  ByteOutStream(uint8_t* const data, const size_t size)
    : ByteStreamBase(data, size)
    , writePtr_(data)
  {}

  void resetw()
  {
    writePtr_ = dataStartPtr_;
  }

  size_t tellp() const
  {
    assert(writePtr_ >= dataStartPtr_);
    return writePtr_ - dataStartPtr_;
  }

  void seekp(const int32_t offset)
  {
    writePtr_ += offset;
  }

  void put(const uint8_t byte)
  {
    assert(writePtr_ + 1 <= dataEndPtr_);
    *writePtr_ = byte;
    writePtr_++;
  }

  void write(const uint8_t* bytes, const size_t size)
  {
    assert(writePtr_ + size <= dataEndPtr_);
    memcpy(writePtr_, bytes, size);
    writePtr_ += size;
  }

  template<typename T>
  void write(const T& t)
  {
    write(reinterpret_cast<const uint8_t*>(&t), sizeof(t));
  }

  bool availablep() const
  {
    return writePtr_ < dataEndPtr_;
  }

protected:
  uint8_t* writePtr_;
};

#endif