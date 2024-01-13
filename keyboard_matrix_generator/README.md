# Keyboard matrix generator

I found this laptop keyboard in an e-waste bin and decided to make use of it. The first step is to figure out the keyboard matrix by scanning with an MCU. The connector is a 32P 1mm FPC. I was only able to find 34P female connectors on aliexpress. But the alignment seems OK.

Model: NBLBJ US RT V0.2
Pins: 32, numbered top down, starting from 1.

MCU: WROOM 32

Columns: 
1, 2, 3, 5, 6, 8, 9, 12
Rows:
4, 7, 10, 11, 13, 14, 16, 17, 18, 20, 21, 22, 23, 24

The rest of the pins are either not used, for the trackpad, or backlight. I don't need any of them. In total, there are 22 pins. I may use a Pico and QMK