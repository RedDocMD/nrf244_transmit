#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <LibPrintf.h>

const int CE = 7;
const int CSN = 8;
const int channel = 34;

RF24 rf(CE, CSN);

uint8_t msg[32];
uint8_t addr[5] = {0x01, 0x02, 0x03, 0x04, 0x05};

void wait_forever() {
  while (true)
    asm volatile ("wfe");
}

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
  rf.setPayloadSize(sizeof(msg));
  rf.setChannel(channel);

  rf.setAddressWidth(sizeof(addr));
  rf.stopListening();
  rf.openWritingPipe(addr);
}

void printMsg() {
  printf("[");
  for (int i = 0; i < sizeof(msg); i++) {
    if (i != 0) printf(", ");
    printf("%02x", msg[i]);
  }
  printf("]");
}

void randMsg() {
  for (int i = 0; i < sizeof(msg); i++) {
    msg[i] = random(256);
  }
}

void loop() {
  randMsg();
  rf.write(msg, sizeof(msg));

  printf("Sent msg: ");
  printMsg();
  printf("\n");

  delay(1000);
}
