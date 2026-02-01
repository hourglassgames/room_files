# Raspberry Pi Pico 2 W Relay Test

This document describes how to wire a Raspberry Pi Pico 2 W to relay boards and provides a simple test program that energizes each relay for 1 second, one at a time.

## Wiring overview

- **Controller**: Raspberry Pi Pico 2 W (MicroPython)
- **Relay board power**: external 5V supply (do **not** power relays from the Pico 5V rail)
- **Control pins**: GP0 through GP22, plus GP26
- **Ground reference**: the Pico GND **must** be tied to the relay board GND (common ground)

### Connections (control side)

Connect each relay input to the corresponding Pico GPIO:

- Relay 1  -> GP0
- Relay 2  -> GP1
- Relay 3  -> GP2
- Relay 4  -> GP3
- Relay 5  -> GP4
- Relay 6  -> GP5
- Relay 7  -> GP6
- Relay 8  -> GP7
- Relay 9  -> GP8
- Relay 10 -> GP9
- Relay 11 -> GP10
- Relay 12 -> GP11
- Relay 13 -> GP12
- Relay 14 -> GP13
- Relay 15 -> GP14
- Relay 16 -> GP15
- Relay 17 -> GP16
- Relay 18 -> GP17
- Relay 19 -> GP18
- Relay 20 -> GP19
- Relay 21 -> GP20
- Relay 22 -> GP21
- Relay 23 -> GP22
- Relay 24 -> GP26

### Power and ground

- External 5V supply **+5V** -> relay board **VCC/JD-VCC** (depends on board)
- External 5V supply **GND** -> relay board **GND**
- Pico **GND** -> relay board **GND** (common ground)

> If your relay board has separate **VCC** and **JD-VCC**, consult its documentation. In most cases for opto-isolated boards, **JD-VCC** powers the relays and **VCC** powers the input side. Ensure grounds are common.

## Software

Install MicroPython on the Pico 2 W, then copy `pico_relay_test.py` to the board as `main.py` (or run it manually).

### Notes on relay logic

Some relay boards are **active-low** (IN pin is driven LOW to energize). Others are **active-high**. The script includes an `ACTIVE_STATE` constant you can flip if the relays toggle the wrong way.

- If relays stay on all the time, set `ACTIVE_STATE = 0`.
- If relays never turn on, set `ACTIVE_STATE = 1`.

---

### File: `pico_relay_test.py`

```
Copy the Python file generated alongside this document.
```
