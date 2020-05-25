# Arduino Calculator
Arduino calculator with some extra sauce

This calculator includes basic functionality (+, - , ×, /, etc) and some extra functions (Factorial, Sin, Cos, Tan, etc). This sketch is built for Arduino Nano. Current keyboard layout looks like this
|   |   |    |    |    |        |
|:-:|:-:|:--:|:--:|:--:|:------:|
| 1 | 2 |  3 |  + |  ^ |   Sin  |
| 4 | 5 |  6 |  - |  √ |   Cos  |
| 7 | 8 |  9 |  * | n! |   Tan  |
| 0 | % |  С |  / | <- |   Pi   |
| = | . | E1 | E2 | BL | Switch |

E1 & E2 buttons are used to switch between numbers; BL button is used to toggle backlight. For more information, see sketch itself
This sketch requires LiquidCrystal_I2C, Keypad and Wire libraries
