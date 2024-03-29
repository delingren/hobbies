// The MIT License (MIT)

// Copyright (c) 2024 Deling Ren

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

namespace synaptics {
void begin(uint8_t clock_pin, uint8_t data_pin, void (*byte_received)(uint8_t));
void write_byte(uint8_t data, int responses = 1);

void reset() { write_byte(0xFF, 3); }

void enable() { write_byte(0xF4, 1); }

void disable() { write_byte(0xF5, 1); }

void set_mode(uint8_t mode) {
  disable();
  write_byte(0xE8);
  write_byte((mode & 0xC0) >> 6);
  write_byte(0xE8);
  write_byte((mode & 0x30) >> 4);
  write_byte(0xE8);
  write_byte((mode & 0x0C) >> 2);
  write_byte(0xE8);
  write_byte((mode & 0x03) >> 0);
  write_byte(0xF3);
  write_byte(0x14);
  enable();
}
} // namespace synaptics