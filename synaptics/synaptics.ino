namespace synaptics {
// Sample code for interacting with a PS2 mouse from a mega32u4.
// Writing is synchronous. Async write is hard for the client to handle.
namespace { // anonymous namespace to hide code from the client.
// On atmel mega32u4, clock_pin needs to be one of these: 0, 1, 2, 3, 7,
// otherwise, we need to use pin change interrupt.
int clock_pin_;
int data_pin_;
void (*byte_received_)(uint8_t);

void pull_low(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void pull_high(uint8_t pin) { pinMode(pin, INPUT_PULLUP); }

uint8_t read_bit() {
  pinMode(data_pin_, INPUT);
  return digitalRead(data_pin_);
}

void write_bit(uint8_t data) {
  pinMode(data_pin_, OUTPUT);
  digitalWrite(data_pin_, data);
}

void wait_clock(uint8_t value) {
  if (value == LOW) {
    pinMode(clock_pin_, INPUT_PULLUP);
  } else {
    pinMode(clock_pin_, INPUT);
  }

  unsigned long millis_start = millis();
  while (digitalRead(clock_pin_) != value) {
    if (millis() - millis_start > 25) {
      Serial.println("wait_clock timed out.");
      return;
    }
  }
}

volatile int receive_index = 0;
volatile uint8_t receive_buffer = 0;
volatile uint8_t parity = 0;

void disable_interrupt() {
  cli();
  // We need to abandon the ongoing read from the device.
  // Otherwise it'll go haywire next time we enable interrupt.
  receive_index = 0;
  receive_buffer = 0;
  parity = 0;
}

void bit_received() {
  int clock = digitalRead(clock_pin_);
  if (clock != LOW) {
    return;
  }

  int bit = read_bit();
  if (receive_index == 0) {
    // Start bit
    if (bit != LOW) {
      Serial.println("Start bit error.");
    }
  } else if (receive_index >= 1 && receive_index <= 8) {
    // Payload bit
    receive_buffer |= bit << (receive_index - 1);
    parity ^= bit;
  } else if (receive_index == 9) {
    // Parity bit
    parity ^= bit;
    if (parity != 1) {
      Serial.println("Parity bit error.");
    }

  } else if (receive_index == 10) {
    // Stop bit
    if (bit != HIGH) {
      Serial.println("Stop bit error.");
    }
    byte_received_(receive_buffer);
    receive_buffer = 0;
    receive_index = 0;
    parity = 0;
    return;
  }

  receive_index++;
}

// Reads a byte synchronously. This should only be called from write_byte() to
// read the response and shouldn't be called by the client.
uint8_t read_byte() {
  // Start bit
  wait_clock(LOW);
  uint8_t start_bit = read_bit();
  if (start_bit != LOW) {
    Serial.println("Start bit error.");
  }
  wait_clock(HIGH);

  uint8_t data = 00;
  uint8_t parity = 0;
  for (int i = 0; i < 8; i++) {
    wait_clock(LOW);
    uint8_t bit = read_bit();
    wait_clock(HIGH);

    data |= bit << i;
    parity ^= bit;
  }

  wait_clock(LOW);
  parity ^= read_bit();
  wait_clock(HIGH);
  if (parity != 1) {
    Serial.println("Parity error.");
  }

  wait_clock(LOW);
  uint8_t stop = read_bit();
  if (stop != 1) {
    Serial.println("Stop bit error.");
  }
  pull_low(clock_pin_);
  delayMicroseconds(50);
  pull_high(clock_pin_);

  return data;
}
} // namespace

void begin(uint8_t clock_pin, uint8_t data_pin,
           void (*byte_received)(uint8_t)) {
  clock_pin_ = clock_pin;
  data_pin_ = data_pin;
  byte_received_ = byte_received;

  pull_high(clock_pin);
  pull_high(data_pin);

  attachInterrupt(digitalPinToInterrupt(clock_pin_), bit_received, FALLING);
}

static void write_byte(uint8_t data, int responses = 1) {
  uint8_t oldSREG = SREG;
  disable_interrupt();

  // Bring CLK low for 100 us.
  pull_low(clock_pin_);
  delayMicroseconds(100);
  // Bring DATA low.
  pull_low(data_pin_);
  // Release CLK
  pull_high(clock_pin_);
  // Now we are in the request-to-send state.

  uint8_t const stop = 1;
  uint8_t parity = 1;

  // bits 0 - 7: payload
  for (int i = 0; i < 8; i++) {
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

  for (int i = 0; i < responses; i++) {
    read_byte();
  }

  SREG = oldSREG;
}

void reset() { write_byte(0xFF, 3); }

void enable() { write_byte(0xF4, 1); }

void disable() { write_byte(0xF5, 1); }
}; // namespace synaptics

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
