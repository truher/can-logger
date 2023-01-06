/*
 * CAN SENDER
 *
 * Send messages for the logger to hear.
 */
#include <FlexCAN_T4.h>
#include "can.h"

static CAN_message_t txMsg;
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;

void setup(void) {
  Serial.begin(9600);
  delay(1000);
  Serial.println("*************************** CAN SENDER ***************************");
  can0.begin();
  can0.setBaudRate(1000000);  // roborio speed is 1mbps
  can0.disableFIFO();
  can0.mailboxStatus();
  //txMsg.id = 0x100;
  txMsg.flags.extended = true;
  txMsg.len = 8;
  txMsg.seq = false;
  txMsg.buf[0] = 10;
  txMsg.buf[1] = 20;
  txMsg.buf[2] = 0;
  txMsg.buf[3] = 100;
  txMsg.buf[4] = 128;
  txMsg.buf[5] = 64;
  txMsg.buf[6] = 32;
  txMsg.buf[7] = 16;
}

void loop(void) {
  can0.events();                   // empty the tx queue a bit
  if (can0.getTXQueueCount() > 0)  // wait until it's empty
    return;

  txMsg.id = 0;
  // broadcast: disable
  frc_id id;
  id.device_number = 0;
  id.api_index = 0;
  id.api_class = 0;
  id.mfr = manufacturer::Broadcast;
  id.type = device_type::Broadcast_Messages;
  id.padding = 0;

  memcpy(&txMsg.id, &id, 4);
  Serial.println(txMsg.id, BIN);
  txMsg.buf[0]++;
  can0.write(txMsg);

  // hunting for the bit fields
  id.device_number = 0;  // should be LSB so id = 1
  id.api_index = 1; // should be bit 6 so 1000000 = 0x40
  id.api_class = 0;
  id.mfr = manufacturer::Broadcast;
  id.type = device_type::Broadcast_Messages;
  id.padding = 0;

  memcpy(&txMsg.id, &id, 4);
  Serial.println(txMsg.id, BIN);
  Serial.println(txMsg.id, HEX);
  txMsg.buf[0]++;
  can0.write(txMsg);

  // some sort of talon status message
  id.device_number = 1;
  id.api_index = 2;
  id.api_class = 5;
  id.mfr = manufacturer::CTR_Electronics;
  id.type = device_type::Motor_Controller;
  id.padding = 0;

  memcpy(&txMsg.id, &id, 4);
  Serial.println(txMsg.id, BIN);
  txMsg.buf[0]++;
  can0.write(txMsg);

  // heartbeat: enabled; api index might be 1 or 2, doc is inconsistent.
  id.device_number = 0;
  id.api_index = 1;
  id.api_class = 6;
  id.mfr = manufacturer::NI;
  id.type = device_type::Robot_Controller;
  id.padding = 0;

  memcpy(&txMsg.id, &id, 4);
  Serial.println(txMsg.id, BIN);
  txMsg.buf[0]++;

  heartbeat h;
  h.RedAlliance = false;
  h.Enabled = true;
  h.Autonomous = false;
  h.Test = false;
  h.WatchdogEnabled = true;
  h.Reserved = false;

  memcpy(&txMsg.buf[4], &h, 1);
  can0.write(txMsg);

  txMsg.buf[0]++;
  can0.write(txMsg);

  txMsg.buf[0]++;
  can0.write(txMsg);
  // for now go slow
  // TODO: remove this
  delay(1000);
}
