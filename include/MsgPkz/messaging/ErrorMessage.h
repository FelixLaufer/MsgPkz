#ifndef _ERROR_MESSAGE_H_
#define _ERROR_MESSAGE_H_

#include "Message.h"
class ErrorMessage : public Message
{
public:
  static const Message::Type type;
  static const size_t size;

  ErrorMessage()
    : ms()
  {}

  ErrorMessage(ByteInStream& is)
    : ms(is)
  {}

  ByteInStream ms;

private:
  virtual void deserialize(ByteInStream& is) override
  {}

  virtual void serialize(ByteOutStream& os) const override
  {}
};

const Message::Type ErrorMessage::type = Message::Type::ERROR;
const size_t ErrorMessage::size = 0;

#endif