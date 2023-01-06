#ifndef PCAPFORMAT_H
#define PCAPFORMAT_H

#include <TimeLib.h>

/*
 * PCAP Format
 *
 * Supplies buffers in pcap format, as defined by Wireshark.
 *
 * See https://wiki.wireshark.org/Development/LibpcapFileFormat
 * See https://docs.wpilib.org/en/stable/docs/software/can-devices/can-addressing.html
 */
class PcapFormat {
  public:
  /*
   * 24-byte file header
   */
  struct GlobalHeader {
    uint32_t magic_number = 0xa1b2c3d4;
    uint16_t version_major = 2;
    uint16_t version_minor = 4;
    int32_t thiszone = 0;
    uint32_t sigfigs = 0;
    // max length of captured packets, in octets
    // TODO: make this reasonable
    uint32_t snaplen = 65535;
    // CAN 2.0b (29-bit), see libpcap/pcap/dlt.h:782
    // (Roborio CAN is 29-bit)
    uint32_t network = 190;
  };

  const GlobalHeader GLOBAL_HEADER;

  /*
   * 16-byte record header
   */
  struct RecordHeader {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
  };

  RecordHeader recordHeader_;

  /*
   * 8-16 bytes
   */
  struct Record {
    RecordHeader header;
    uint32_t id : 29;
    bool err : 1;
    bool rtr : 1;
    bool eff : 1;  // should end up as MSB
    uint8_t len: 8; // max 8 bytes
    uint32_t padding: 24; // zero
    uint64_t  data; // max 8 bytes
  };

  Record record_;

  /*
   * Returns the global header as a 24-byte buffer.
   */
  const uint8_t* getGlobalHeader() {
    return (const uint8_t*)&GLOBAL_HEADER;
  }

  /*
  * Returns a record header as a 16-byte buffer.
  * Note there's just one of these, so don't hang on to it.
  *
  * data_len: bytes in the packet
  */
   uint8_t* newRecordHeader(uint32_t data_len) {
    uint32_t timestamp = now();
    uint32_t microseconds = (unsigned int)(micros() - millis() * 1000);
    recordHeader_ = { timestamp, microseconds, data_len, data_len };
    return ( uint8_t*)&recordHeader_;
  }

  uint8_t* newRecord(uint32_t id, const uint8_t* buf, uint8_t count) {
    uint32_t timestamp = now();
    uint32_t microseconds = (unsigned int)(micros() - millis() * 1000);
    recordHeader_ = { timestamp, microseconds, (uint32_t)count+8, (uint32_t)count+8 };
    record_ = { recordHeader_, id, false, false, true, count, 0, 0};
    memcpy(&record_.data, buf, count);
    return ( uint8_t*)&record_;
  }
};

#endif  // PCAPFORMAT_H
