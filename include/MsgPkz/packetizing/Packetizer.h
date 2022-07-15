#ifndef _PACKETIZER_H_
#define _PACKETIZER_H_

#include "COBS.h"
#include "SLIP.h"
#include "../messaging/Message.h"
#include "../utils/memory/Buffer.h"
#include "../utils/memory/ByteStream.h"
#include "../utils/meta/FunctionTraits.h"
#include "../utils/meta/FunctorMatching.h"

#define FOLD(X) (void)std::initializer_list<int>{ (X, 0)... }

template <size_t N, typename Tuple>
using nth_type_of_tuple = typename std::tuple_element<N, Tuple>::type;

template <typename Tuple>
constexpr size_t typeTupleSize()
{
  return std::tuple_size<std::remove_reference_t<Tuple>>::value;
}

template <typename Tuple>
constexpr auto typeTupleIndexSequence() -> decltype(std::make_index_sequence<typeTupleSize<Tuple>()>{})
{
  return std::make_index_sequence<typeTupleSize<Tuple>()>{};
}

class Packetizer
{
public:
  using TTranscoder = COBS;
  static constexpr uint8_t PACKET_DELIMITER{ TTranscoder::PACKET_DELIMITER };
  static constexpr uint8_t MAX_MESSAGE_SIZE{ 50 };
  static constexpr uint8_t MAX_MESSAGES{ 7 };

  Packetizer()
    : serS_(serBuf_.data(), serBuf_.size())
    , encS_(encBuf_.data(), encBuf_.size())
    , decS_(decBuf_.data(), decBuf_.size())
  {}

  ~Packetizer() = default;

  template <typename... TMessages>
  ByteStream packetize(TMessages&&... messages)
  {
    serS_.resetw();
    FOLD(messages.serialize(serS_));

    encS_.resetw();
    ByteInStream ms(serS_.data(), serS_.tellp());
    TTranscoder::encode(ms, encS_);

    return ByteStream(encS_.data(), encS_.tellp());
  }

  template <typename... TPacketCallbacks>
  void depacketize(ByteStream& ps, TPacketCallbacks&&... packetCallbacks)
  {
    decS_.resetw();
    TTranscoder::decode(ps, decS_,
      [&](ByteInStream& ms)
      {
        while (ms.availableg())
        {
          using CallbackTypeTuples = argument_types_tuple<TPacketCallbacks...>;
          const size_t sizeBefore = ms.tellg();
          processPacket<CallbackTypeTuples>(ms, std::forward<TPacketCallbacks>(packetCallbacks)...);
          const size_t sizeAfter = ms.tellg();
          if (!(sizeBefore < sizeAfter))
            break;
        }
      },
      [&](ByteInStream& ms)
      {
        matching_functor<ErrorMessage>(packetCallbacks..., [](const ErrorMessage&) {})
        (
          std::move(ErrorMessage(ms))
        );
      }
    );
  }

protected:
  template <typename TMessageType>
  bool checkMessageType(ByteInStream& ms)
  {
    const bool valid = ms.peek<typename TMessageType::Type>() == TMessageType::type;
    ms.seekg(TMessageType::size);
    return valid;
  }

  template <typename TMessageTypesTuple, size_t... N>
  bool checkMessageTypes(ByteInStream& ms, std::index_sequence<N...>)
  {
    bool valid = true;
    FOLD((valid = valid && checkMessageType<nth_type_of_tuple<N, TMessageTypesTuple>>(ms)));
    return valid;
  }

  template <typename TMessageTypesTuple>
  bool checkMessageTypes(ByteInStream& ms)
  {
    const size_t gBefore = ms.tellg();
    const bool valid = checkMessageTypes<TMessageTypesTuple>(ms, typeTupleIndexSequence<TMessageTypesTuple>());
    const size_t gAfter = ms.tellg();
    ms.seekg(-static_cast<int32_t>(gAfter - gBefore));
    return valid && gAfter == ms.size();
  }

  template <typename TMessageTypesTuple, size_t... N>
  TMessageTypesTuple deserializeMessages(ByteInStream& ms, std::index_sequence<N...>)
  {
    return TMessageTypesTuple{ std::move(nth_type_of_tuple<N, TMessageTypesTuple>(ms))... };
  }

  template <typename TMessageTypesTuple>
  TMessageTypesTuple deserializeMessages(ByteInStream& ms)
  {
    return deserializeMessages<TMessageTypesTuple>(ms, typeTupleIndexSequence<TMessageTypesTuple>());
  }

  template <typename TMessageTypesTuple, typename... TPacketCallbacks>
  void deserializeAndInvoke(ByteInStream& ms, TPacketCallbacks&&... packetCallbacks)
  {
    if (ms.availableg() && checkMessageTypes<TMessageTypesTuple>(ms))
    {
      std::apply
      (
        std::move(matching_functor<TMessageTypesTuple>(std::forward<TPacketCallbacks>(packetCallbacks)...)),
        std::move(deserializeMessages<TMessageTypesTuple>(ms))
      );
    }
  }

  template <typename TCallbackTypeTuples, size_t... N, typename... TPacketCallbacks>
  void processPacket(ByteInStream& ms, std::index_sequence<N...>, TPacketCallbacks&&... packetCallbacks)
  {
    FOLD((deserializeAndInvoke<nth_type_of_tuple<N, TCallbackTypeTuples>>(ms, std::forward<TPacketCallbacks>(packetCallbacks)...)));
  }

  template <typename TCallbackTypeTuples, typename... TPacketCallbacks>
  void processPacket(ByteInStream& ms, TPacketCallbacks&&... packetCallbacks)
  {
    processPacket<TCallbackTypeTuples>(ms, typeTupleIndexSequence<TCallbackTypeTuples>(), std::forward<TPacketCallbacks>(packetCallbacks)...);
  }

  Buffer<MAX_MESSAGE_SIZE * MAX_MESSAGES> serBuf_;
  ByteOutStream decS_;
  Buffer<MAX_MESSAGE_SIZE * MAX_MESSAGES> encBuf_;
  ByteOutStream serS_;
  Buffer<MAX_MESSAGE_SIZE * MAX_MESSAGES> decBuf_;
  ByteOutStream encS_;
};

#undef FOLD

#endif
