#ifndef _BYTE_IN_STREAM_H_
#define _BYTE_IN_STREAM_H_

#include "ByteStreamBase.h"

class ByteInStream : virtual public ByteStreamBase
{
public:
  ByteInStream()
    : ByteStreamBase()
    , readPtr_(nullptr)
  {}

  ByteInStream(uint8_t* const data, const size_t size)
    : ByteStreamBase(data, size)
    , readPtr_(data)
  {}

  ByteInStream(const ByteStreamBase& bs)
    : ByteStreamBase(bs)
    , readPtr_(bs.data())
  {}

  void resetg()
  {
    readPtr_ = dataStartPtr_;
  }

  size_t tellg() const
  {
    assert(readPtr_ >= dataStartPtr_);
    return readPtr_ - dataStartPtr_;
  }

  void seekg(const int32_t offset)
  {
    readPtr_ += offset;
  }

  void skip(const uint32_t offset)
  {
    seekg(offset);
  }

  void get(uint8_t& byte)
  {
    assert(readPtr_ + 1 <= dataEndPtr_);
    byte = *readPtr_;
    readPtr_++;
  }

  uint8_t get()
  {
    uint8_t ret;
    get(ret);
    return ret;
  }

  void read(uint8_t* bytes, const size_t size)
  {
    assert(readPtr_ + size <= dataEndPtr_);
    memcpy(bytes, readPtr_, size);
    readPtr_ += size;
  }

  template <typename T>
  T read()
  {
    T t{};
    read(reinterpret_cast<uint8_t*>(&t), sizeof(t));
    return t;
  }

  template<typename T>
  void read(T& data)
  {
    data = read<T>();
  }

  void peek(uint8_t& byte) const
  {
    assert(readPtr_ + 1 <= dataEndPtr_);
    byte = *readPtr_;
  }

  template <typename T = uint8_t>
  T peek() const
  {
    assert(readPtr_ + sizeof(T) <= dataEndPtr_);
    T t{};
    memcpy(&t, readPtr_, sizeof(T));
    return t;
  }

  void peek(uint8_t* bytes, const size_t size) const
  {
    assert(readPtr_ + size <= dataEndPtr_);
    memcpy(bytes, readPtr_, size);
  }

  bool availableg() const
  {
    return readPtr_ < dataEndPtr_;
  }

protected:
  uint8_t* readPtr_;
};

#endif