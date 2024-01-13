#include <Arduino.h>

const int pins = 32;

int cols[pins] = {15, 2, 4, 0, 17, 5, 0, 19, 21, 0, 0, 13, 0, 0, 0, 0,
                  0,  0, 0, 0,  0,  0, 0,  0,  0,  0, 0, 0,  0, 0, 0, 0};
int rows[pins] = {0,  0,  0, 16, 0,  0,  18, 0,  0, 22, 23, 0, 12, 14, 0, 26,
                  25, 33, 0, 32, 35, 34, 39, 36, 0, 0,  0,  0, 0,  0,  0,  0};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting to scan...");

  delay(200);
}

void loop() {
  for (int col = 0; col < pins; col++) {
    if (cols[col] <= 0) {
      continue;
    }
    pinMode(cols[col], OUTPUT);
    digitalWrite(cols[col], HIGH);
    for (int row = 0; row < pins; row++) {
      if (rows[row] <= 0) {
        continue;
      }
      pinMode(rows[row], INPUT_PULLDOWN);
      int value = digitalRead(rows[row]);
      if (value == HIGH) {
        Serial.print(col + 1);
        Serial.print(" - ");
        Serial.print(row + 1);
        Serial.println();
      }
    }
    digitalWrite(cols[col], LOW);
  }
  delay(100);
}
