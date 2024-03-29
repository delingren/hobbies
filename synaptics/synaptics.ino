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

#include "src/synaptics.h"
// #include <HID.h>
// PS2 protocol: https://wiki.osdev.org/PS/2_Mouse

// void hid_init() {
//   static const uint8_t hidReportDescriptor[] PROGMEM = {

//       //  Mouse
//       0x05, 0x01, // USAGE_PAGE (Generic Desktop)  // 54
//       0x09, 0x02, // USAGE (Mouse)
//       0xa1, 0x01, // COLLECTION (Application)
//       0x09, 0x01, //   USAGE (Pointer)
//       0xa1, 0x00, //   COLLECTION (Physical)
//       0x85, 0x01, //     REPORT_ID (1)
//       0x05, 0x09, //     USAGE_PAGE (Button)
//       0x19, 0x01, //     USAGE_MINIMUM (Button 1)
//       0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
//       0x15, 0x00, //     LOGICAL_MINIMUM (0)
//       0x25, 0x01, //     LOGICAL_MAXIMUM (1)
//       0x95, 0x03, //     REPORT_COUNT (3)
//       0x75, 0x01, //     REPORT_SIZE (1)
//       0x81, 0x02, //     INPUT (Data,Var,Abs)
//       0x95, 0x01, //     REPORT_COUNT (1)
//       0x75, 0x05, //     REPORT_SIZE (5)
//       0x81, 0x03, //     INPUT (Cnst,Var,Abs)
//       0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
//       0x09, 0x30, //     USAGE (X)
//       0x09, 0x31, //     USAGE (Y)
//       0x09, 0x38, //     USAGE (Wheel)
//       0x15, 0x81, //     LOGICAL_MINIMUM (-127)
//       0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
//       0x75, 0x08, //     REPORT_SIZE (8)
//       0x95, 0x03, //     REPORT_COUNT (3)
//       0x81, 0x06, //     INPUT (Data,Var,Rel)
//       0xc0,       //   END_COLLECTION
//       0xc0,       // END_COLLECTION
//   };
//   static HIDSubDescriptor node(hidReportDescriptor,
//                                sizeof(hidReportDescriptor));
//   HID().AppendDescriptor(&node);
// }

// void hid_report(uint8_t buttons, int8_t x, int8_t y) {
//   uint8_t m[4];
//   m[0] = buttons;
//   m[1] = x;
//   m[2] = y;
//   m[3] = 0; // wheel
//   HID().SendReport(1, m, sizeof(m));
// }

int8_t to_hid_value(bool overflow, bool negative, uint8_t data) {
  // HID uses [-127, 127]. I.e. an 8-bit signed integer, except -128.
  // PS2 uses [-256, 255]. I.e. a 9-bit signed integer.
  int16_t ps2_value = negative ? (overflow ? -256 : ((int16_t)data) - 256)
                               : (overflow ? 255 : data);
  return negative ? (-127 <= ps2_value && ps2_value <= -1 ? ps2_value : -127)
                  : (0 <= ps2_value && ps2_value <= 127 ? ps2_value : 127);
}

volatile uint64_t pending_packet;
volatile bool has_pending_packet = false;

void packet_received(uint64_t data) {
  has_pending_packet = true;
  pending_packet = data;
}

void process_pending_packet() {
  uint64_t data = pending_packet;
  has_pending_packet = false;

  uint16_t y =
      (data >> 40) & 0x00FF | (data >> 4) & 0x0F00 | (data >> 17) & 0x1000;
  uint16_t x =
      (data >> 32) & 0x00FF | (data >> 0) & 0x0F00 | (data >> 16) & 0x1000;
  uint8_t z = (data >> 16) & 0xFF;
  uint8_t w = (data >> 26) & 0x01 | (data >> 2) & 0x06;

  bool right = (data >> 1) & 0x01;
  bool left = (data >> 0) & 0x01;

  static char output[256];
  sprintf(output, "x: %u, y: %u, z: %u, w: %02x", x, y, z, w);
  Serial.println(output);
}

void byte_received(uint8_t data) {
  // Serial.println(data, HEX);
  static uint64_t buffer = 0;
  static int index = 0;
  buffer |= ((uint64_t)data) << index;
  index += 8;
  if (index == 48) {
    packet_received(buffer);
    index = 0;
    buffer = 0;
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  // hid_init();

  synaptics::begin(0, 9, byte_received);
  synaptics::reset();
  // synaptics::enable();
  // absolute, low rate, w=1
  synaptics::set_mode(0x81);
}

void loop() {
  if (has_pending_packet) {
    process_pending_packet();
  }
}
