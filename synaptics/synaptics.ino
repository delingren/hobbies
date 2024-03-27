// Sample code for interacting with a PS2 mouse from a mega32u4, using interrupt based read and write.

// On atmel mega32u4, clock_pin needs to be one of these: 0, 1, 2, 3, 7, otherwise, use pin change interrupt
int clock_pin = 7;
int data_pin = 9;

bool sending = false;
uint8_t send_buffer;

void pull_low(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void pull_high(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
}

bool initialize() {
  sending = false;
  pull_high(clock_pin);
  pull_high(data_pin);
  
  attachInterrupt(digitalPinToInterrupt(clock_pin), bit_received, FALLING);
  start_transmit(0xFF);

  // 0xFA and 0xAA bytes are separated by up to 500 ms of calibration delay
  // Plus the normal response time, 550 should be good enough.
  delay(550);
  return true;
}

void start_transmit(uint8_t data)
{
  while (sending) {
    delay(4);
  }
  
  cli();

  sending = true;
  send_buffer = data;

  // 1. Bring CLK low for 100 us.
  pull_low(clock_pin);
  delayMicroseconds(100);

  // 2. Bring DATA low.
  pull_low(data_pin);

  // 3. Release CLK
  pull_high(clock_pin);

  sei();
}

void byte_received (uint8_t data) {
  Serial.println(data, HEX);
}

void bit_received() {
  int clock = digitalRead(clock_pin);
  if (clock != LOW) {
    return;
  }

  if (sending) {
    uint8_t bit;
    static volatile uint8_t parity = 1;
    static volatile int send_index = 0;

    if (send_index >= 0 && send_index <= 7) {
      // Payload bit
      bit = (send_buffer >> send_index) & 0x01;
      parity ^= bit;
      send_index ++;
    } else if (send_index == 8) {
      // Parity bit
      bit = parity;
      parity = 1;
      send_index ++;
    } else if (send_index == 9) {
      // Stop bit
      bit = 1;
      send_index ++;
    } else {
      // Line control. Release DATA
      bit = 1;
      sending = false;
      send_index = 0;
    }

    if (bit != 0) {
      pull_high(data_pin);
    }
    else {
      pull_low(data_pin);
    }
  }
  else {
    int bit = digitalRead(data_pin);

    static volatile int receive_index = 0;
    static volatile uint8_t receive_buffer = 0;

    if (receive_index == 10)
    {
      // Stop bit
      byte_received(receive_buffer);
      receive_buffer = 0;
      receive_index = 0;
      return;
    }

    if (receive_index >= 1 && receive_index <= 8) {
      // Payload bit
      receive_buffer |= bit << (receive_index - 1);
    }

    receive_index++;
  }  
}

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  Serial.println("Resetting the touchpad...");
  initialize();
  delay(1000);
}
