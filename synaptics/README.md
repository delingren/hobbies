Packet formats

Relative mode (mouse compatible)
   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
(1)| Yo| Xo| Ys| Xs| 1 | M | R | L |
(2)|               X               |
(3)|               Y               |

Absolute mode, Wmode = 0
   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
(1)| 1 | 0 |Fin|N/A| 0 |Ges| R | L |
(2)|     Y11-Y8    |     X11-X8    |
(3)|             Z7-Z0             |
(4)| 1 | 1 |Y12|X12| 0 |Ges| R | L |
(5)|             X7-X0             |
(6)|             Y7-Y0             |

Fin: virtual button click (tap)
Ges: tap & slide gesture

Absolute mode, Wmode = 1
   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
(1)| 1 | 0 | W3| W2| 0 | W1| R | L |
(2)|     Y11-Y8    |     X11-X8    |
(3)|             Z7-Z0             |
(4)| 1 | 1 |Y12|X12| 0 | W0|R^D|L^U|
(5)|             X7-X0             |
(6)|             Y7-Y0             |

W values
Value    | Capability     | Interpretation
W = 0    | capMultiFinger | Two fingers on the pad
W = 1    | capMultiFinger | Three or more fingers on the pad
W = 2    | capEWmode      | Extended W mode
W = 3    | capPassThru    | Pass-Through encapsulation packet (see section 5.1)
W = 4–15 | capPalmDetect  | Finger width; 15 is the maximum reportable width