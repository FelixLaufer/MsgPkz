#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "../utils/arx/XPlattform.h"
#include "../utils/memory/ByteInStream.h"
#include "../utils/memory/ByteOutStream.h"
#include "DataTypes.h"
class Message
{
public:
  enum Type : uint8_t
  {
    STATUS = 0x00,
    FRAME = 0x01,
    IMU = 0x02,
    KINEMATICS = 0x03,
    JOINT_ANGLES = 0x04,
    ERROR = 0xFE,
    UNKNOWN = 0xFF
  };

  static const Type type;
  static const size_t size;

protected:
  Message() = default;
  virtual ~Message() = default;

  virtual void deserialize(ByteInStream& is) = 0;
  virtual void serialize(ByteOutStream& os) const = 0;
};

const Message::Type Message::type = Message::Type::UNKNOWN;
const size_t Message::size = 0;

// Convenience includes
#include "StatusMessage.h"
#include "FrameMessage.h"
#include "IMUMessage.h"
#include "ErrorMessage.h"

#endif