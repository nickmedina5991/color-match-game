# Color Match: Embedded RGB Color Mixing Game

An interactive electronics game for the Arduino Mega 2560 that teaches kids how screens make every color by mixing red, green, and blue light.

I built it for the Society of Hispanic Professional Engineers (SHPE) booth at the **Second Annual Queen City Clean Waters Fair** (September 25, 2026, Hoyt Lake, Delaware Park, Buffalo, NY), where about 30 kids ages 4 to 12 played it.

<!-- Add photos to docs/images/ and uncomment:
![Kids playing Color Match at the fair](docs/images/booth.jpg)
![Close up of the LEDs and score display](docs/images/closeup.jpg)
-->

## How it plays

| Part | What it does |
|---|---|
| **LED 1 (Secret)** | Shows a secret color |
| **LED 2 (Yours)** | Shows the color the player mixes with 3 knobs: red, green, blue |
| **LED 3 (Meter)** | Stays off until the player is close enough, then chimes and pulses red |
| **Button** | Submits a guess. A match plays a victory song and a rainbow light show |
| **4 digit display** | Score, victory words (`YAY`, `EPIC`, `nICE`, `PrO`...), `OOPS` on a miss, and the high score |

**Levels**

| Level | Score | Colors | Match window (out of 255) |
|---|---|---|---|
| 1 | 0 to 4 | Red, green, blue, yellow, cyan, magenta | 110 |
| 2 | 5 to 9 | Same 6 colors | 85 (tighter) |
| 3 | 10 and up | All 13, adding white and half strength mixes like orange, pink, and purple | 110 |

After 45 seconds with nobody playing, the game returns to an attract mode (rainbow LEDs, a spinning animation, `PLAY`, and the high score) for the next visitor.

## Firmware highlights

- **Interrupt driven display.** Timer5 is configured at the register level in CTC mode (16 MHz / 64 prescaler / 250 counts) to fire a 1 kHz interrupt that multiplexes the 4 digit 7 segment display. The display stays flicker free even while blocking songs and light shows run.
- **Timer planning.** All 6 ATmega2560 hardware timers are in use, so LED pins were chosen to avoid conflicts:

  | Timer | Used for |
  |---|---|
  | Timer0 | `millis()`, PWM on pin 4 |
  | Timer1 | PWM on pins 11, 12 |
  | Timer2 | `tone()` for the buzzer (so LEDs avoid pins 9 and 10) |
  | Timer3 | PWM on pins 2, 3, 5 |
  | Timer4 | PWM on pins 6, 7, 8 |
  | Timer5 | 1 kHz display refresh interrupt |

- **Gamma corrected PWM.** 9 PWM channels use a 2.2 gamma curve so brightness changes look even to the human eye.
- **Input conditioning.** The 3 analog knobs use exponential smoothing with dead zones at both ends of travel, and the button uses a 30 ms debounce.
- **State machine.** Attract and playing states, 3 difficulty levels, random victory songs, and victory words that never repeat back to back.
- **Power on self test.** At every boot the display lights all 28 segments and each LED cycles red, green, blue, so wiring faults show up during booth setup.
- **Serial telemetry.** Knob values, the secret color, the distance from a match, the score, and the level print at 9600 baud for debugging.

## Hardware

| Qty | Part |
|---|---|
| 1 | Arduino Mega 2560 |
| 3 | RGB LEDs, common cathode |
| 9 | 330 Ω resistors (one per LED color leg) |
| 3 | 10 kΩ linear potentiometers (B103) |
| 1 | Push button |
| 1 | Passive buzzer |
| 1 | 4 digit 7 segment display, common cathode (5461AS) |
| 8 | 1 kΩ resistors (display segments; 7 if the decimal point is left unwired) |
| 1 | Breadboard and jumper wires |

## Wiring

### LEDs, knobs, button, buzzer

| Part | Connection |
|---|---|
| LED 1 (Secret) R / G / B | pins 2 / 3 / 4, each through 330 Ω |
| LED 2 (Yours) R / G / B | pins 5 / 6 / 7, each through 330 Ω |
| LED 3 (Meter) R / G / B | pins 8 / 11 / 12, each through 330 Ω |
| LED common (longest) legs | GND rail |
| Knob wipers (middle legs) | A0 (red), A1 (green), A2 (blue) |
| Knob outer legs | 5V rail and GND rail |
| Button | pin 22 and GND (internal pull up, no resistor needed) |
| Buzzer | plus leg to pin 24, other leg to GND |

RGB LED leg order varies between batches. Trust the startup self test: each LED should flash red, then green, then blue.

### 4 digit display

Hold the display with the decimal points at the bottom; pin 1 is bottom left. The display wires in order around the double header at the end of the Mega: the bottom row runs along the even pins, and the top row comes back along the odd pins.

```
Display pin:   1   2   3   4   5   6     (bottom row, left to right)
Mega pin:     42  44  46  48  50  52

Display pin:  12  11  10   9   8   7     (top row, left to right)
Mega pin:     43  45  47  49  51  53
```

- Pins 1, 2, 3, 4, 5, 7, 10, 11 are segments: one 1 kΩ resistor each
- Pins 6, 8, 9, 12 are digit commons: wire them directly
- Pin 3 is the decimal point and is never lit, so it is optional

## Uploading

1. Open `Color_Match_Game/Color_Match_Game.ino` in the Arduino IDE.
2. Select **Tools > Board > Arduino Mega or Mega 2560** and the correct port.
3. Upload. No extra libraries are needed.
4. Optional: open the Serial Monitor at 9600 baud.

## Settings

All tuning values are at the top of the sketch:

| Setting | Purpose |
|---|---|
| `TOLERANCE_LEVEL1/2/3` | Match window per level; bigger is easier |
| `IDLE_MS` | Time before returning to attract mode |
| `METER_PULSE` | Color of the "you found it" pulse |
| `COMMON_ANODE` | Set `true` for common anode RGB LEDs |
| `SEG_COMMON_ANODE` | Set `true` for a common anode display |

## Troubleshooting

`tools/Wiring_Test/Wiring_Test.ino` checks the buzzer, all 9 LED channels, the knobs, and the button one at a time, and reports each step over the Serial Monitor.

| Symptom | Likely cause |
|---|---|
| Knob readings drift on their own or rise and fall together | Breadboard power rails not connected to the Mega's 5V and GND |
| LEDs only light while the button is pressed | LED ground rail not connected to the Mega's GND |
| One LED shows the wrong colors | Two of its color legs are swapped; check the startup self test |
| Board resets during sounds | Low battery or weak power supply; use a fresh 7 to 9 V supply |
| Display segments inverted or dark | Display is common anode; set `SEG_COMMON_ANODE = true` |

## License

MIT. See [LICENSE](LICENSE).
