from machine import Pin
import time

# =======================
# CONFIGURATION
# =======================

# Keypad wiring (as provided)
KEYPAD_COLS = [5, 4, 3, 2]   # GPIOs connected to column lines
KEYPAD_ROWS = [9, 8, 7, 6]   # GPIOs connected to row lines

# External LED pin (CHANGE THIS if needed)
LED_PIN = 15                 # External LED on GPIO 15

# Relay pin
RELAY_PIN = 14               # Active HIGH assumed

# 5-digit unlock code
CODE_LENGTH = 5
CORRECT_CODE = "21471"

# LED blink timing
BLINK_ON_MS  = 80
BLINK_OFF_MS = 80

# =======================
# HARDWARE SETUP
# =======================

# Rows as outputs
rows = [Pin(pin_num, Pin.OUT) for pin_num in KEYPAD_ROWS]

# Columns as inputs with internal pull-up
cols = [Pin(pin_num, Pin.IN, Pin.PULL_UP) for pin_num in KEYPAD_COLS]

# External LED and relay
led = Pin(LED_PIN, Pin.OUT)
relay = Pin(RELAY_PIN, Pin.OUT)

# Start safe
for r in rows:
    r.value(1)

led.value(0)
relay.value(0)

# Typical 4x4 keypad map
KEY_MAP = [
    ['1', '2', '3', 'A'],
    ['4', '5', '6', 'B'],
    ['7', '8', '9', 'C'],
    ['*', '0', '#', 'D'],
]

# =======================
# HELPER FUNCTIONS
# =======================

def blink(times, on_ms=BLINK_ON_MS, off_ms=BLINK_OFF_MS):
    for _ in range(times):
        led.value(1)
        time.sleep_ms(on_ms)
        led.value(0)
        time.sleep_ms(off_ms)

def scan_key():
    """Scan keypad; return key char or None."""
    for row_index, row_pin in enumerate(rows):
        row_pin.value(0)  # activate row

        for col_index, col_pin in enumerate(cols):
            if col_pin.value() == 0:  # active low = key pressed
                key = KEY_MAP[row_index][col_index]

                # Debounce: wait until released
                while col_pin.value() == 0:
                    time.sleep_ms(10)

                row_pin.value(1)
                time.sleep_ms(20)
                return key

        row_pin.value(1)

    return None

# =======================
# MAIN LOOP
# =======================

entered_code = ""

print("Keypad lock system running...")
print("Waiting for 5-digit code...")

while True:
    key = scan_key()

    if key is None:
        time.sleep_ms(10)
        continue

    # Only accept numeric keys
    if not key.isdigit():
        continue

    # Append digit
    entered_code += key
    blink(1)  # flash once per digit

    # When 5 digits entered:
    if len(entered_code) == CODE_LENGTH:
        blink(3)  # 3 blinks for code entry complete

        if entered_code == CORRECT_CODE:
            print("Correct code! Unlocking...")
            led.value(1)     # LED stays on
            relay.value(1)   # Relay latches on (unlock)

            # Lock stays unlocked until reset
            while True:
                time.sleep(1)

        else:
            print("Incorrect code:", entered_code)
            entered_code = ""
            time.sleep_ms(200)
