#ifndef _IMU_MESSAGE_H_
#define _IMU_MESSAGE_H_

#include "Message.h"
class IMUMessage : public Message
{
public:
  static const Message::Type type;
  static const size_t size;

  IMUMessage(const uint8_t id, const uint32_t timestamp, const Vec3& acc, const Vec3& gyr, const Vec3& mag)
    : id(id)
    , timestamp(timestamp)
    , acc(acc)
    , gyr(gyr)
    , mag(mag)
  {}

  IMUMessage()
    : IMUMessage(0xFF, 0, Vec3(), Vec3(), Vec3())
  {}

  IMUMessage(ByteInStream& is)
    : IMUMessage()
  {
    deserialize(is);
  }

  virtual void deserialize(ByteInStream& is) override
  {
    uint8_t type;
    is.read(type);
    if (type != IMUMessage::type)
      return;

    is.read(id);
    is.read(timestamp);
    is.read(acc);
    is.read(gyr);
    is.read(mag);
  }

  virtual void serialize(ByteOutStream& os) const override
  {
    os.write(IMUMessage::type);
    os.write(id);
    os.write(timestamp);
    os.write(acc);
    os.write(gyr);
    os.write(mag);
  }

  uint8_t id;
  uint32_t timestamp;
  Vec3 acc;
  Vec3 gyr;
  Vec3 mag;
};

const Message::Type IMUMessage::type = Message::Type::IMU;
const size_t IMUMessage::size = 42;

#endif