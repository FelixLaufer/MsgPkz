#ifndef _BYTE_STREAM_H_
#define _BYTE_STREAM_H_

#include "ByteStreamBase.h"
#include "ByteInStream.h"
#include "ByteOutStream.h"

class ByteStream : public ByteInStream, public ByteOutStream
{
public:
  ByteStream()
    : ByteStreamBase()
    , ByteInStream()
    , ByteOutStream()
  {}

  ByteStream(const ByteStreamBase& bs)
    : ByteStreamBase(bs)
    , ByteInStream(bs)
    , ByteOutStream(bs)
  {}

  ByteStream(uint8_t* const data, const size_t size)
    : ByteStreamBase(data, size)
    , ByteInStream(data, size)
    , ByteOutStream(data, size)
  {}

  void reset()
  {
    resetg();
    resetw();
  }
};

#endif