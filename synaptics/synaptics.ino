#include "src/synaptics.h"

void byte_received(uint8_t data) { Serial.println(data, HEX); }

void setup() {
  Serial.begin(115200);
  delay(1000);
  synaptics::begin(7, 9, byte_received);
}

void loop() {
  Serial.println("Resetting ...");
  synaptics::reset();
  Serial.println("Enabling ...");
  synaptics::enable();
  delay(2000);
}
