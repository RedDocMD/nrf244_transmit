#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

RF24 rf;
char msg[4];
char addr[4] = "abc";
int i;
int mxm = 1000;

void wait_forever() {
  while (true)
    asm volatile ("wfe");
}

void setup() {
  Serial.begin(9600);
  printf_begin();

  if (!rf.begin()) {
    Serial.println("NRF24 setup failed");
    wait_forever();
  }
  rf.setAutoAck(false);
  rf.setPayloadSize(sizeof(msg));

  rf.setAddressWidth(sizeof(addr) - 1);
  rf.stopListening();
  rf.openWritingPipe((const uint8_t *)addr);

  i = 0;
}

void loop() {
  i %= mxm;
  snprintf(msg, sizeof(msg), "%03d", i);
  rf.write(msg, sizeof(msg));
  Serial.print("Sent msg: ");
  Serial.println(msg);
  ++i;
  delay(1000);
}
