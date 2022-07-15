#ifndef _FRAME_MESSAGE_H_
#define _FRAME_MESSAGE_H_

#include "Message.h"
class FrameMessage : public Message
{
public:
  static const Message::Type type;
  static const size_t size;

  FrameMessage(const uint32_t timestamp)
    : timestamp(timestamp)
  {}

  FrameMessage()
    : FrameMessage(0)
  {}

  FrameMessage(ByteInStream& is)
    : FrameMessage()
  {
    deserialize(is);
  }

  virtual void deserialize(ByteInStream& is) override
  {
    uint8_t type;
    is.read(type);
    if (type != FrameMessage::type)
      return;

    is.read(timestamp);
  }

  virtual void serialize(ByteOutStream& os) const override
  {
    os.write(FrameMessage::type);
    os.write(timestamp);
  }

  uint32_t timestamp;
};

const Message::Type FrameMessage::type = Message::Type::FRAME;
const size_t FrameMessage::size = 5;

#endif