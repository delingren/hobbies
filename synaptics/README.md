# Laptop Touchpad Project

I pulled a touchpad out of an old laptop and wanted to reuse it. Most touchpads implement PS2. This particular one (Synaptics) also has its own extended features such as scrolling. But these extended features require specific drivers rather than the standard PS2 driver. Fortunately I found a interfacing [guide](touchpad_RevB.pdf) published by Synaptics. For the standard PS2 protocol, there are many sources. I find [this one](https://wiki.osdev.org/PS/2_Mouse) concise and helpful.

My idea is to use a microcontroller to interact with the touchpad and the host (e.g. a PC). In particular, I have a [DFRobot Beetle](https://www.dfrobot.com/product-1075.html) which uses an ATmega32U4 chip, and is compatible with Arduino Leonardo or the popular Pro Micro.

## Goal

Turn this touchpad into a mouse that supports:
* Regular two-button mouse functionalities. I.e. cursor movements, left and right clicks.
* Two-finger scrolling.
* Optionally: two finder click as right click.
* Optionally: tap as click.

## Touchpad info

* Pulled from an HP Envy Sleekbook 6.
* Synaptics chip # T1320A. I wasn't able to find any datasheet.
* Physical pinout. With help from this [video](https://www.youtube.com/watch?v=XdznW0ZuzGo), I figured out the pinout:
![Pinout](IMG_0835.jpeg)
* I did a few programmatic queries and here are the results.
* Query `0x00: 01 47 18`:  
`Version 8.1`
* Query `0x01: 20 87 40`:  
`Model 0x0887`
* Query `0x02: D0 01 23`:
```
Model sub number: 01
Middle button: no
Pass through: no
Low power: no
Multi finger report: yes
Multi finger detect: yes
Can sleep: no
Palm detect: yes
```

## Interfacing with the touchpad

### Reading and writing bytes
A PS2 mouse can operate in either the stream mode or the remote mode. The latter is basically the polling mode where the device only responds to polls from the host. The stream mode is where the device reports data whenever there's any movement. We want to use the stream mode. Synaptics' guide has very detailed information on the circuit level information on PS2 and its own specifics.

Generally, on the controller, data reading is done through interrupts, i.e. asynchronously. I initially implemented writing in a similar way. However, I found it very clumsy. The main reason is that I need to keep track of the writing process itself. On top of that, every write is followed by reading one or more bytes of response. The result is that there are so many states to keep track. In the end, I decided to implement writing in a synchronous manner. Since we don't expect to recieve data while writing (writing and reading are done on the same CLK/DATA line), there isn't any point in writing asynchronously. We also read the responses of a write synchronously.

### Packet formats

#### Relative mode
This is compatible with generic PS/2 mice. Each packet consists of 3 bytes.

```
   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
(1)| Yo| Xo| Ys| Xs| 1 | M | R | L |
(2)|               X               |
(3)|               Y               |
```

#### Absolute mode
In order to take advantage of extended features of the touchpad, such as pressure value, multi-finger reporting, etc, we need to operate in the so called absolute mode. In this mode, each packet consists of 6 bytes.

* Wmode = 0
```
   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
(1)| 1 | 0 |Fin|N/A| 0 |Ges| R | L |
(2)|     Y11-Y8    |     X11-X8    |
(3)|             Z7-Z0             |
(4)| 1 | 1 |Y12|X12| 0 |Ges| R | L |
(5)|             X7-X0             |
(6)|             Y7-Y0             |
```

Fin: virtual button click (tap)
Ges: tap & slide gesture

* Wmode = 1
```
   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
(1)| 1 | 0 | W3| W2| 0 | W1| R | L |
(2)|     Y11-Y8    |     X11-X8    |
(3)|             Z7-Z0             |
(4)| 1 | 1 |Y12|X12| 0 | W0|R^D|L^U|
(5)|             X7-X0             |
(6)|             Y7-Y0             |
```

W values
Value    | Capability     | Interpretation
W = 0    | capMultiFinger | Two fingers on the pad
W = 1    | capMultiFinger | Three or more fingers on the pad
W = 2    | capEWmode      | Extended W mode
W = 3    | capPassThru    | Pass-Through encapsulation packet (see section 5.1)
W = 4–15 | capPalmDetect  | Finger width; 15 is the maximum reportable width

## Interfacing with host