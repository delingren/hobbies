// Sample code for interacting with a PS2 mouse from a mega32u4, using interrupt based read and write.

class ps2 {
  public:
  static ps2 touchpad;

  // On atmel mega32u4, clock_pin needs to be one of these: 0, 1, 2, 3, 7, otherwise, we need to use pin change interrupt
  int clock_pin;
  int data_pin;

  void pull_low(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  void pull_high(uint8_t pin) {
    pinMode(pin, INPUT_PULLUP);
  }

  uint8_t read_bit() {
    pinMode(data_pin, INPUT);
    return digitalRead(data_pin);
  }

  void write_bit(uint8_t data) {
    pinMode(data_pin, OUTPUT);
    digitalWrite(data_pin, data);
  }

  void wait_clock(uint8_t value) {
    if (value == LOW) {
      pinMode(clock_pin, INPUT_PULLUP);
    } else {
      pinMode(clock_pin, INPUT);
    }

    unsigned long millis_start = millis();
    while (digitalRead(clock_pin) != value) {
      if (millis() - millis_start > 25) {
        Serial.println("wait_clock timed out.");
        return;
      }
    }
  }

  void begin(uint8_t clock_pin, uint8_t data_pin) {
    this->clock_pin = clock_pin;
    this->data_pin = data_pin;

    pull_high(clock_pin);
    pull_high(data_pin);
  }

  uint8_t read_byte() {
    pull_high(clock_pin);

    // Start bit
    wait_clock(LOW);
    uint8_t start_bit = read_bit();
    if (start_bit != LOW) {
      Serial.println("Start bit error.");
    }
    wait_clock(HIGH);

    uint8_t data = 0x00;
    uint8_t parity = 0x01;
    for (int i = 0; i < 8; i ++) {
      wait_clock(LOW);
      uint8_t bit = read_bit();
      wait_clock(HIGH);

      data |= bit << i;
      parity ^= bit;
    }

    wait_clock(LOW);
    parity ^= read_bit();
    wait_clock(HIGH);
    if (parity != 0) {
      Serial.println("Parity error.");
    }

    wait_clock(LOW);
    uint8_t stop = read_bit();
    if (stop != 1) {
      Serial.println("Stop bit error.");
    }
    pull_low(clock_pin);
    delayMicroseconds(50);

    return data;
  }

  void write_byte(uint8_t data){
    cli();

    // Bring CLK low for 100 us.
    pull_low(clock_pin);
    delayMicroseconds(100);
    // Bring DATA low.
    pull_low(data_pin);
    // Release CLK
    pull_high(clock_pin);
    // Now we are in the request-to-send state.

    uint8_t const stop = 1;
    uint8_t parity = 1;

    // bits 0 - 7: payload
    for (int i = 0; i < 8; i ++) {
      // Get the LSB
      uint8_t bit = data & 0x01;
      // Update parity
      parity ^= bit;
      // Shift data for next iteration
      data >>= 1;
    
      // Device samples when CLK is low.
      wait_clock(LOW);
      write_bit(bit);
      wait_clock(HIGH);
    }

    // bit 8: parity
    wait_clock(LOW);
    write_bit(parity);
    wait_clock(HIGH);

    // bit 9: stop
    wait_clock(LOW);
    write_bit(stop);
    wait_clock(HIGH);

    // bit 10: line control
    wait_clock(LOW);
    uint8_t line_control = read_bit();
    wait_clock(HIGH);
    if (line_control != LOW) {
      Serial.println("Line control error.");
    }

    sei();

    uint8_t ack = read_byte();
    if (ack != 0xFA) {
      Serial.print("Ack error ");
      Serial.print(ack, HEX);
    }
  }

  void reset() {
    write_byte(0xFF);
    uint8_t res1 = read_byte();
    uint8_t res2 = read_byte();
    if (res1 != 0xAA || res2 != 0x00) {
      Serial.print("Wrong response from reset command ");
      Serial.print(res1, HEX);
      Serial.print(' ');
      Serial.println(res2, HEX);
    }
  }  
};

ps2 ps2::touchpad;

void setup() {
  Serial.begin(115200);
  delay(1000);
  ps2::touchpad.begin(7, 9);
}

void loop() {
  Serial.println("Resetting the touchpad...");
  ps2::touchpad.reset();

  delay(1000);
}
