#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "../arx/XPlattform.h"

template <size_t SIZE>
class Buffer
{
public:
  Buffer()
    : data_()
  {}

  uint8_t* const data() const
  {
    return const_cast<uint8_t* const>(data_);
  }

  constexpr size_t size() const
  {
    return SIZE;
  }

protected:
  uint8_t data_[SIZE];
};

#endif