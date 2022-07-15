#ifndef _SLIP_H_
#define _SLIP_H_

#include "../utils/memory/ByteInStream.h"
#include "../utils/memory/ByteOutStream.h"
#include "CRC.h"

class SLIP
{
public:
  static constexpr uint8_t PACKET_DELIMITER = 0xC0;

  static void encode(ByteInStream& is, ByteOutStream& os)
  {
    const uint8_t* isData = is.data();
    const size_t isSize = is.size();

    uint8_t* const osData = os.data();
    size_t osSize = 0;

    for (size_t i = 0; i < isSize; ++i)
    {
      assert(osSize < os.size());
      osData[osSize] = isData[i];

      if (isData[i] == MARKER_END)
      {
        assert(osSize + 1 < os.size());
        osData[osSize] = MARKER_ESC;
        osData[++osSize] = MARKER_ESC_END;
      }
      else if (isData[i] == MARKER_ESC)
      {
        assert(osSize + 1 < os.size());
        osData[++osSize] = MARKER_ESC_ESC;
      }
      
      osSize++;
    }

    const uint16_t crc = crc16CCITT1021(isData, isSize);

    isData = reinterpret_cast<const uint8_t*>(&crc);
    for (size_t i = 0; i < 2; ++i)
    {
      assert(osSize < os.size());
      osData[osSize] = isData[i];

      if (isData[i] == MARKER_END)
      {
        assert(osSize + 1 < os.size());
        osData[osSize] = MARKER_ESC;
        osData[++osSize] = MARKER_ESC_END;
      }
      else if (isData[i] == MARKER_ESC)
      {
        assert(osSize + 1 < os.size());
        osData[++osSize] = MARKER_ESC_ESC;
      }

      osSize++;
    }

    assert(osSize < os.size());
    osData[osSize++] = MARKER_END;
    os.seekp(osSize);
  }

  template <typename FuncPacketCallback, typename FuncErrorCallback>
  static void decode(ByteInStream& is, ByteOutStream& os, FuncPacketCallback&& packetCallback, FuncErrorCallback&& errorCallback)
  {
    const uint8_t* const isData = is.data();
    const size_t isSize = is.size();

    uint8_t* const osData = os.data();
    size_t osSize = 0;

    for (size_t i = 0; i < isSize; ++i)
    {
      if (isData[i] == MARKER_END)
        continue;
      if (isData[i] == MARKER_ESC)
      {
        assert(i + 1 < isSize);
        if (isData[i + 1] == MARKER_ESC_END)
        {
          assert(osSize < os.size());
          osData[osSize++] = MARKER_END;
        }
        else if (isData[i + 1] == MARKER_ESC_ESC)
        {
          assert(osSize < os.size());
          osData[osSize++] = MARKER_ESC;
        }
        else
        {
          errorCallback(is);
          return;
        }
        ++i;
      }
      else
      {
        assert(osSize < os.size());
        osData[osSize++] = isData[i];
      }
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
  static constexpr uint8_t MARKER_END = PACKET_DELIMITER;
  static constexpr uint8_t MARKER_ESC = 0xDB;
  static constexpr uint8_t MARKER_ESC_END = 0xDC;
  static constexpr uint8_t MARKER_ESC_ESC = 0xDD;

  SLIP() = delete;
};

#endif