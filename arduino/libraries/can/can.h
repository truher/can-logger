#ifndef CAN_H
#define CAN_H

#include <TimeLib.h>

enum class device_type : uint8_t {
  Broadcast_Messages = 0,
  Robot_Controller = 1,
  Motor_Controller = 2,
  Relay_Controller = 3,
  Gyro_Sensor = 4,
  Accelerometer = 5,
  Ultrasonic_Sensor = 6,
  Gear_Tooth_Sensor = 7,
  Power_Distribution_Module = 8,
  Pneumatics_Controller = 9,
  Miscellaneous = 10,
  IO_Breakout = 11,
  Firmware_Update = 31
};

enum class manufacturer : uint8_t {
  Broadcast = 0,
  NI = 1,
  Luminary_Micro = 2,
  DEKA = 3,
  CTR_Electronics = 4,
  REV_Robotics = 5,
  Grapple = 6,
  MindSensors = 7,
  Team_Use = 8,
  Kauai_Labs = 9,
  Copperforge = 10,
  Playing_With_Fusion = 11,
  Studica = 12
};

enum class broadcast : uint8_t {
  Disable = 0,
  System_Halt = 1,
  System_Reset = 2,
  Device_Assign = 3,
  Device_Query = 4,
  Heartbeat = 5,
  Sync = 6,
  Update = 7,
  Firmware_Version = 8,
  Enumerate = 9,
  System_Resume = 10
};

/*
 * heartbeat bits for byte 4
 */
struct __attribute__((__packed__)) heartbeat {
  bool RedAlliance : 1;
  bool Enabled : 1;
  bool Autonomous : 1;
  bool Test : 1;
  bool WatchdogEnabled : 1;
  uint8_t Reserved : 3;
};

/*
 * See https://docs.wpilib.org/en/stable/docs/software/can-devices/can-addressing.html
 * Note packed attribute is required here for byte-spanning bit fields
 */
struct __attribute__((__packed__)) frc_id {
  uint8_t device_number : 6;
  uint8_t api_index : 4;
  uint8_t api_class : 6;
  manufacturer mfr : 8;
  device_type type : 5;  // this produces a "too small" warning which can be ignored
  uint8_t padding : 3;
};

/*
 * PCAP Format
 *
 * Supplies buffers in pcap format, as defined by Wireshark.
 *
 * See https://wiki.wireshark.org/Development/LibpcapFileFormat
 */
class PcapFormat {
public:
  /*
   * 24-byte file header
   */
  struct __attribute__((__packed__)) GlobalHeader {
    uint32_t magic_number = 0xa1b2c3d4;
    uint16_t version_major = 2;
    uint16_t version_minor = 4;
    int32_t thiszone = 0;
    uint32_t sigfigs = 0;
    // max length of captured packets, in octets
    // TODO: make this reasonable
    uint32_t snaplen = 65535;
    // CAN 2.0b (29-bit), see libpcap/pcap/dlt.h:782
    uint32_t network = 190;
  };

  const GlobalHeader GLOBAL_HEADER;

  /*
   * 16-byte record header
   */
  struct __attribute__((__packed__)) RecordHeader {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;   
    uint32_t orig_len;
  };

  RecordHeader recordHeader_;

  /*
   * 8-16 bytes
   */
  struct __attribute__((__packed__)) Record {
    RecordHeader header;
    uint32_t id : 29;
    bool err : 1;
    bool rtr : 1;
    bool eff : 1;           // should end up as MSB
    uint8_t len : 8;        // max 8 bytes
    uint32_t padding : 24;  // zero
    uint8_t data[8];
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
    return (uint8_t*)&recordHeader_;
  }

  uint8_t* newRecord(uint32_t id, const uint8_t* buf, uint8_t count) {
    uint32_t timestamp = now();
    uint32_t microseconds = (unsigned int)(micros() - millis() * 1000);
    recordHeader_ = { timestamp, microseconds, (uint32_t)count + 8, (uint32_t)count + 8 };
    record_ = { recordHeader_, id, false, false, true, count, 0, 0 };
    memcpy(&record_.data, buf, count);
    return (uint8_t*)&record_;
  }
};

#endif  // CAN_H
