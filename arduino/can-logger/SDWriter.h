#ifndef SDWRITER_H
#define SDWRITER_H

#include "SdFat.h"
#include "RingBuf.h"  // NOTE: this is the SdFat RingBuf, not Locoduino.

// Use Teensy SDIO
#define SD_CONFIG SdioConfig(FIFO_SDIO)
//#define SD_CONFIG  SdioConfig(DMA_SDIO)

// Size to log 10 byte lines at 25 kHz for more than ten minutes.
#define LOG_FILE_SIZE 10 * 20000 * 600  // 150,000,000 bytes.

// Space to hold more than 800 ms of data for 10 byte lines at 25 ksps.
#define RING_BUF_CAPACITY 400 * 512
#define LOG_FILENAME "canlog"

#define BLOCK_SIZE 512

/*
 * Write to SD using a ring buffer.
 *
 * See SdFat/examples/TeensySdioLogger/TeensySdioLogger.ino
 */
class SDWriter {

public:
  SdFs sd;
  FsFile file;

  // RingBuf for File type FsFile.
  RingBuf<FsFile, RING_BUF_CAPACITY> rb;

  bool ready = false;

  SDWriter() {
    if (!sd.begin(SD_CONFIG)) {
      Serial.println("sd begin failed\n");
      return;
    }

    if (!openFile()) {
      Serial.println("file open failed\n");
      return;
    }

    // File must be pre-allocated to avoid huge
    // delays searching for free clusters.
    if (!file.preAllocate(LOG_FILE_SIZE)) {
      Serial.println("preAllocate failed\n");
      file.close();
      return;
    }
    rb.begin(&file);
    ready = true;
  }

  /*
   * Writes one block from the ring buffer to SD, if a block
   * is available to write, and if the card is not busy.
   *
   * Call this from loop().
   *
   * Returns number of bytes written.
   *
   * TODO: close the file when full, open a new one
   */
  size_t writeBlock() {
    if (!ready) return 0;
    if (rb.bytesUsedIsr() < BLOCK_SIZE) return 0;  // not yet a full block
    if (file.isBusy()) return 0;                   // not yet ok to write
    return rb.writeOut(BLOCK_SIZE);
  }

  /*
   * Writes to the ring buffer.
   *
   * Call this from the interrupt handler.
   *
   * Returns the number of bytes written.
   */
  size_t log(const void* buf, size_t count) {
    if (!ready) return 0;
    if (count > rb.bytesFreeIsr()) return 0;  // buffer full
    return rb.memcpyIn(buf, count);
  }

  /*
   * Open a unique file.
   */
  bool openFile() {

    //searches for the next non-existent file name
    int c = 0;
    String filename = "/" + (String)LOG_FILENAME + ".pcap";
    while (sd.exists(filename)) {
      filename = "/" + (String)LOG_FILENAME + "_" + (String)c + ".pcap";
      c++;
    }

    if (!file.open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC)) {
      Serial.println("file open failed\n");
      return false;
    }

    Serial.println("opened: " + filename);
    return true;
  }
};

#endif  // SDWRITER_H
