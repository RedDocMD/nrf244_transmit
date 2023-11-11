#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <LibPrintf.h>
#include <Gaussian.h>
#include "hid.h"

constexpr int CE = 7;
constexpr int CSN = 8;
constexpr int channel = 34;

RF24 rf(CE, CSN);

uint8_t msg[32];
uint8_t addr[5] = {0x01, 0x02, 0x03, 0x04, 0xCD};

struct Payload {
  uint8_t dev_type;
  uint8_t pkt_type;
  uint8_t model;
  uint8_t _unk;
  uint16_t seq;
  uint8_t flags;
  uint8_t meta;
  uint8_t _pad1;
  uint8_t hid_code;
  uint8_t _pad2[5];
  uint8_t cksum;
};

Payload *pay = (Payload *)msg;

void wait_forever() {
  while (true)
    asm volatile ("wfe");
}

void init_payload() {
  pay->dev_type = 0x0A;
  pay->pkt_type = 0x38;
  pay->model = 0x06;
  pay->seq = 0;
  pay->flags = 0xFF;
  pay->meta = 0x00;
}

#ifndef WPM
#define WPM 100
#endif

constexpr int WPM_TO_CPM = 5;
constexpr int CPM = WPM * WPM_TO_CPM;
constexpr int MS_IN_MIN = 60 * 1000;

constexpr int AVG_DELAY = MS_IN_MIN / CPM;
constexpr int DELAY_VAR = 20;

Gaussian delay_distr(AVG_DELAY, DELAY_VAR);

constexpr int BURST_MIN = 30;
constexpr int BURST_MAX = 50;
constexpr int BURST_DELAY = 50;

int burst_cnt, burst_lim;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));

  if (!rf.begin()) {
    printf("NRF24 setup failed\n");
    wait_forever();
  }

  rf.setAutoAck(false);
  rf.setPALevel(RF24_PA_MIN);
  rf.setDataRate(RF24_2MBPS);
  rf.setPayloadSize(sizeof(Payload));
  rf.setChannel(channel);

  init_payload();

  rf.setAddressWidth(sizeof(addr));
  rf.stopListening();
  rf.openWritingPipe(addr);

  burst_cnt = 0;
  burst_lim = random(BURST_MIN, BURST_MAX + 1);
}

void print_msg() {
  printf("[");
  for (int i = 0; i < sizeof(msg); i++) {
    if (i != 0) printf(", ");
    printf("%02x", msg[i]);
  }
  printf("]");
}

int get_delay() {
  int delay = delay_distr.random();
  if (burst_cnt == burst_lim) {
    delay += BURST_DELAY;
    burst_cnt = 0;
    burst_lim = random(BURST_MIN, BURST_MAX + 1);
  }
  return delay;
}

char random_ch() {
  constexpr int TOT = 26 + 10 + 1;
  int r = random(TOT);
  if (r == 0) return ' ';
  else if (r <= 10) return ('0' + r - 1);
  else return ('a' + r - 11);
}

void loop() {
  ++pay->seq;
  char ch = random_ch();
  pay->hid_code = hid_reverse(ch);

  printf("%c", ch);
  rf.write(msg, sizeof(Payload));

  // printf("Sent msg: ");
  // print_msg();
  // printf("\n");

  ++burst_cnt;
  // printf("%d\n", WPM);
  delay(get_delay());
}
