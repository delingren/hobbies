/*
This code is written for atmega32u4. It may work for other avr chips.
*/

int clock_pin = 8;
// int clock_pin = 3;
int data_pin = 9;
bool sending = false;
uint8_t send_buffer;

void pull_low(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void pull_high(int pin) {
  pinMode(pin, INPUT_PULLUP);
  // digitalWrite(pin, HIGH);
}

bool initialize() {
  cli();
  sending = false;
  pull_high(clock_pin);
  pull_high(data_pin);

  
  if (digitalPinToInterrupt(clock_pin) != NOT_AN_INTERRUPT) {
    // External interrupt
    attachInterrupt(digitalPinToInterrupt(clock_pin), bit_received, FALLING);
  } else if (digitalPinToPCICR(clock_pin) != 0) {
    // Pin change interrupt
    *(digitalPinToPCICR(clock_pin)) |= 1 << digitalPinToPCICRbit(clock_pin);
    *(digitalPinToPCMSK(clock_pin)) |= 1 << digitalPinToPCMSKbit(clock_pin);
  } else {
    // clock pin is not an interrupt pin
    return false;
  }

  sei();
  startTransmit(0xFF);
  return true;
}

void enableDevice() {
  startTransmit(0xF4);
}

void startTransmit(uint8_t data)
{
  while (sending) {}
  
  uint8_t oldSREG = SREG;
  cli();

  sending = true;
  send_buffer = data;

  // 1. Bring CLK low for 100 us.
  pull_low(clock_pin);
  _delay_ms(100);

  // 2. Bring DATA low.
  pull_low(data_pin);

  // 3. Release CLK
  pull_high(clock_pin);

  SREG = oldSREG;
}

void byte_received (uint8_t data) {
  Serial.println(data, HEX);
}

// Pin change interrupt ISR
ISR(PCINT0_vect) {
  bit_received();
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
  delay(10);

  initialize();
  delay(10);

  enableDevice();
  delay(10);
}

void loop() {
  initialize();
  delay(100);

  enableDevice();
  delay(2000);
}
