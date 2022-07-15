#ifndef _STATUS_MESSAGE_H_
#define _STATUS_MESSAGE_H_

#include "Message.h"
class StatusMessage : public Message
{
public:
  static const Message::Type type;
  static const size_t size;

  StatusMessage(const uint8_t id)
    : id(id)
  {}

  StatusMessage()
    : StatusMessage(0xFF)
  {}

  StatusMessage(ByteInStream& is)
    : StatusMessage()
  {
    deserialize(is);
  }

  virtual void deserialize(ByteInStream& is) override
  {
    uint8_t type;
    is.read(type);
    if (type != StatusMessage::type)
      return;

    is.read(id);
  }

  virtual void serialize(ByteOutStream& os) const override
  {
    os.write(StatusMessage::type);
    os.write(id);
  }

  uint8_t id;
};

const Message::Type StatusMessage::type = Message::Type::STATUS;
const size_t StatusMessage::size = 2;

#endif