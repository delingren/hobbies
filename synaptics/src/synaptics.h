namespace synaptics {
void begin(uint8_t clock_pin, uint8_t data_pin, void (*byte_received)(uint8_t));
void write_byte(uint8_t data, int responses = 1);

void reset() { write_byte(0xFF, 3); }

void enable() { write_byte(0xF4, 1); }

void disable() { write_byte(0xF5, 1); }
} // namespace synaptics