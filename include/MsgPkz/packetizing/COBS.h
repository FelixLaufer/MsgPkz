#ifndef _COBS_H_
#define _COBS_H_

#include "../utils/memory/ByteInStream.h"
#include "../utils/memory/ByteOutStream.h"
#include "CRC.h"

class COBS
{
public:
  static constexpr uint8_t PACKET_DELIMITER = 0x00;

  static void encode(ByteInStream& is, ByteOutStream& os)
  {
    const uint8_t* isData = is.data();
    const size_t isSize = is.size();

    uint8_t* const osData = os.data();
    size_t osSize = 0;

    size_t nextZeroIdx = osSize;
    uint8_t nextZeroCnt = 1;
    assert(osSize < os.size());
    osData[osSize++] = PACKET_DELIMITER;

    for (size_t i = 0; i < isSize; ++i)
    {
      if (nextZeroCnt == PACKET_DELIMITER)
      {
        nextZeroCnt = 1;
        nextZeroIdx = osSize;
        assert(osSize < os.size());
        osData[osSize++] = PACKET_DELIMITER;
      }

      if (isData[i] != PACKET_DELIMITER)
      {
        assert(osSize < os.size());
        osData[osSize++] = isData[i];
        nextZeroCnt++;
      }
      else
      {
        osData[nextZeroIdx] = nextZeroCnt;
        nextZeroCnt = 1;
        nextZeroIdx = osSize;
        assert(osSize < os.size());
        osData[osSize++] = PACKET_DELIMITER;
      }

      if (nextZeroCnt == FRAME_POINTER)
      {
        osData[nextZeroIdx] = nextZeroCnt;
        nextZeroCnt = PACKET_DELIMITER;
        nextZeroIdx = osSize;
      }
    }

    const uint16_t crc = crc16CCITT1021(isData, isSize);

    isData = reinterpret_cast<const uint8_t*>(&crc);
    for (size_t i = 0; i < 2; ++i)
    {
      if (nextZeroCnt == PACKET_DELIMITER)
      {
        nextZeroCnt = 1;
        nextZeroIdx = osSize;
        assert(osSize < os.size());
        osData[osSize++] = PACKET_DELIMITER;
      }

      if (isData[i] != PACKET_DELIMITER)
      {
        assert(osSize < os.size());
        osData[osSize++] = isData[i];
        nextZeroCnt++;
      }
      else
      {
        osData[nextZeroIdx] = nextZeroCnt;
        nextZeroCnt = 1;
        nextZeroIdx = osSize;
        assert(osSize < os.size());
        osData[osSize++] = PACKET_DELIMITER;
      }

      if (nextZeroCnt == FRAME_POINTER)
      {
        osData[nextZeroIdx] = nextZeroCnt;
        nextZeroCnt = PACKET_DELIMITER;
        nextZeroIdx = osSize;
      }
    }

    osData[nextZeroIdx] = nextZeroCnt;
    assert(osSize < os.size());
    osData[osSize++] = PACKET_DELIMITER;    
    os.seekp(osSize);
  }

  template <typename FuncPacketCallback, typename FuncErrorCallback>
  static void decode(ByteInStream& is, ByteOutStream& os, FuncPacketCallback&& packetCallback, FuncErrorCallback&& errorCallback)
  {
    const uint8_t* const isData = is.data();
    const size_t isSize = is.size();

    uint8_t* const osData = os.data();
    size_t osSize = 0;

    uint8_t nextZero = FRAME_POINTER;
    uint8_t rest = 0;

    for (size_t n = 0; n < isSize; ++n)
    {
      const uint8_t data = isData[n];
      if (rest == 0)
      {
        if (nextZero != FRAME_POINTER)
        {
          rest = nextZero = data;
          if (nextZero == PACKET_DELIMITER)
            break;

          assert(osSize < os.size());
          osData[osSize++] = PACKET_DELIMITER;
        }
        else
        {
          rest = nextZero = data;
          if (nextZero == PACKET_DELIMITER)
            break;
        }
      }
      else
      {
        assert(osSize < os.size());
        osData[osSize++] = data;
      }

      --rest;
    }

    if (osSize > sizeof(uint16_t))
    {
      const size_t playloadSize = osSize - sizeof(uint16_t);
      const uint16_t crcCalc = crc16CCITT1021(osData, playloadSize);
      const uint16_t crcSent = *reinterpret_cast<const uint16_t*>(osData + playloadSize);
      if (crcCalc == crcSent)
      {
        ByteInStream packet(osData, playloadSize);
        packetCallback(packet);
      }
      else
        errorCallback(is);
    }
    else
      errorCallback(is);
  }

private:
  static constexpr uint8_t FRAME_POINTER = 0xFF;

  COBS() = delete;
};

#endif