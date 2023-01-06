/*
 * CAN LOGGER
 *
 * Log received messages.  This seems to be able to read 800kbps.
 */
#include "FlexCAN_T4.h"
#include "can.h"
#include "SDWriter.h"

bool ledState = false;
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;
int packets = 0;
SDWriter writer;
PcapFormat format;

void setup(void) {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.println(F("*************************** CAN LOGGER ***************************"));
  can0.begin();
  can0.setBaudRate(1000000);  // roborio speed is 1mbps
  can0.disableFIFO();
  can0.onReceive(canSniff);
  can0.enableMBInterrupts();
  can0.mailboxStatus();

  const uint8_t* header = format.getGlobalHeader();
  writer.log(header, 24);
}

void loop(void) {
  can0.events();
  writer.writeBlock();
}

void canSniff(const CAN_message_t& msg) {
  packets++;
  const uint8_t* pkt = format.newRecord(msg.id, msg.buf, msg.len);
  writer.log(pkt, msg.len+24); // len + 1B len + 3B pad + 4B id + 16B pcap header

  if (packets % 100 != 0)  // too much printing overflows the rx buffer
    return;
  Serial.printf("ct %d buf %d MB %d  OVERRUN: %d  LEN: %d EXT: %d TS: %d ID: %x",
                packets, can0.getRXQueueCount(), msg.mb, msg.flags.overrun, msg.len,
                msg.flags.extended, msg.timestamp, msg.id);
  Serial.print(" Buffer: ");
  for (uint8_t i = 0; i < msg.len; i++) {
    Serial.printf("%x ", msg.buf[i]);
  }
  Serial.println();
  digitalWrite(LED_BUILTIN, ledState);
  ledState ^= 1;
}
