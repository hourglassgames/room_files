# ====== CONFIG ======
WIFI_SSID = "HGG Private"
WIFI_PASSWORD = "HGGamespass10!"

MQTT_BROKER = "192.168.1.101"   # e.g., your Mosquitto broker on LAN
MQTT_PORT = 1883
MQTT_CLIENT_ID = "Candymonium Pico2W Entryway"
MQTT_TOPIC_BUTTON = "Candymonium Audio Buttons"
# Optional: also publish key presses
PUBLISH_KEYPRESSES = False
MQTT_TOPIC_KEY = "Candymonium Entryway Code"

# 3 unique 4-digit codes. They can be entered in any order.
# The lights fill in left to right as codes are completed:
#   1st correct code -> light 1 stays on
#   2nd correct code -> lights 1 and 2 stay on
#   3rd correct code -> all three stay on, then the relay fires
CODES = ["6742", "9984", "0368"]

# Once all codes have been entered, the relay switches ON and stays ON
# until the Pico is powered off or reset.

# Light feedback
KEYPRESS_FLASH_MS = 100   # brief flash on every key press
WRONG_CODE_BLINKS = 3     # quick blinks when a wrong (or repeated) code is entered

# ====== PIN ASSIGNMENTS (change if needed) ======
# Keypad: run keypad_test.py once and paste the two lines it prints here.
# KEYPAD_ROWS = GPIOs for keypad rows R1..R4 (the 1-2-3-A row first).
#               These are driven LOW one at a time while scanning.
# KEYPAD_COLS = GPIOs for keypad columns C1..C4 (the 1-4-7-* column first).
#               These are INPUTS with internal PULL-UPs (pressed = LOW).
KEYPAD_ROWS = [9, 8, 7, 6]
KEYPAD_COLS = [5, 4, 3, 2]

# NOTE: GP12, GP13 and GP14 failed pin_check.py on this board (stuck even with
# nothing attached), so light 3, the relay and the button moved to GP15-GP17.
# Don't use GP12, GP13 or GP14 for anything.

# Three status LEDs (through resistors to GND). These go HIGH to turn on.
# Listed in the order they light up: light 1, light 2, light 3.
LED_PINS = [21,20,19]          # physical pins 14, 15, 20

# Relay control pin (to a relay module).
RELAY_PIN = 16                   # physical pin 21
# Most relay boards are "active-low": they switch ON when the input is pulled LOW.
# With True, the Pico pulls the pin LOW for ON and lets go of it (no drive) for OFF,
# which also works for 5V boards that a 3.3V HIGH can't switch off.
# Set to False for a board that switches ON when its input is HIGH.
# (output_test.py tells you which: True if the relay clicks ON at step 4,
#  False if it clicks ON at step 6.)
RELAY_ACTIVE_LOW = True

# Separate button pin that publishes an MQTT message when pressed
# Wire button to GND; use internal PULL_UP; pressed = LOW
BUTTON_PIN = 17                  # physical pin 22
# =====================

import time
import network
from machine import Pin
try:
    from umqtt.simple import MQTTClient
except Exception as e:
    MQTTClient = None

# ----- Wi-Fi -----
def wifi_connect(ssid, password, timeout=15):
    import network
    wlan = network.WLAN(network.STA_IF)
    if not wlan.active():
        wlan.active(True)
    if not wlan.isconnected():
        wlan.connect(ssid, password)
        t0 = time.ticks_ms()
        while not wlan.isconnected() and (time.ticks_diff(time.ticks_ms(), t0) < timeout * 1000):
            time.sleep(0.2)
    return wlan.isconnected()

# ----- MQTT -----
class MqttWrapper:
    def __init__(self, client_id, host, port=1883):
        self.host = host
        self.port = port
        self.client = MQTTClient(client_id, host, port=port) if MQTTClient else None
        self.connected = False

    def connect(self):
        if not self.client:
            print("umqtt.simple not available in this firmware.")
            return False
        try:
            self.client.connect()
            self.connected = True
            return True
        except Exception as e:
            print("MQTT connect failed:", e)
            self.connected = False
            return False

    def publish(self, topic, msg):
        if self.client and self.connected:
            try:
                self.client.publish(topic, msg)
            except Exception as e:
                # attempt a single reconnect
                print("MQTT publish failed:", e)
                try:
                    self.client.connect()
                    self.client.publish(topic, msg)
                except Exception as e2:
                    print("MQTT republish failed:", e2)

# ----- GPIO setup -----
# Active-LOW keypad scanning. The Pico 2 W's RP2350 chip has a known bug with
# internal PULL-DOWNs (erratum E9): an input that gets pulled high can stay
# stuck reading 1 afterwards. Internal PULL-UPs are not affected, so:
#   - columns are inputs with pull-ups (read 1 when nothing is pressed)
#   - rows sit as harmless inputs until scanned, then the scanned row is driven LOW
#   - a pressed key in that row pulls its column LOW
# Idle rows are never driven, so pressing two keys at once can't short two pins.
row_pins = [Pin(p, Pin.IN, Pin.PULL_UP) for p in KEYPAD_ROWS]
col_pins = [Pin(p, Pin.IN, Pin.PULL_UP) for p in KEYPAD_COLS]

leds = [Pin(p, Pin.OUT, value=0) for p in LED_PINS]

# Relay starts OFF. For an active-low board, "off" = pin not driven, with the
# pull-up holding it high so the board's input can't drift on.
relay = Pin(RELAY_PIN, Pin.IN, Pin.PULL_UP)

def relay_on():
    if RELAY_ACTIVE_LOW:
        relay.init(Pin.OUT, value=0)
    else:
        relay.init(Pin.OUT, value=1)

def relay_off():
    if RELAY_ACTIVE_LOW:
        relay.init(Pin.IN, Pin.PULL_UP)
    else:
        relay.init(Pin.OUT, value=0)

relay_off()

button = Pin(BUTTON_PIN, Pin.IN, Pin.PULL_UP)

# ----- Key map (4x4) -----
# KEYS[row][col] - row index matches KEYPAD_ROWS, col index matches KEYPAD_COLS
KEYS = [
    ['1','2','3','A'],
    ['4','5','6','B'],
    ['7','8','9','C'],
    ['*','0','#','D']
]

# We'll only accept digits into the 4-digit buffer. Other keys can be ignored or used as clear.
DIGITS = set(list("0123456789"))

# ----- Progress + lights -----
achieved = []          # codes completed so far, in the order they were entered
_flash_off_at = None   # when the current key-press flash should end (ticks_ms)

def set_all_leds(state):
    for l in leds:
        l.value(1 if state else 0)

def show_progress():
    """Lights 1..N on for N completed codes, the rest off. Ends any flash."""
    global _flash_off_at
    _flash_off_at = None
    done = len(achieved)
    for i, led in enumerate(leds):
        led.value(1 if i < done else 0)

def start_keypress_flash():
    """Brief flash on a key press. Lights already earned just stay on."""
    global _flash_off_at
    set_all_leds(True)
    _flash_off_at = time.ticks_add(time.ticks_ms(), KEYPRESS_FLASH_MS)

def service_flash():
    """Call often: ends the key-press flash once its time is up."""
    if _flash_off_at is not None and time.ticks_diff(time.ticks_ms(), _flash_off_at) >= 0:
        show_progress()

def blink_unearned(times=WRONG_CODE_BLINKS, on_ms=120, off_ms=120):
    """Wrong-code signal: blink only the lights not yet earned; earned ones stay solid."""
    show_progress()
    time.sleep_ms(150)   # short gap so it doesn't merge with the key-press flash
    unearned = leds[len(achieved):]
    for _ in range(times):
        for led in unearned:
            led.value(1)
        time.sleep_ms(on_ms)
        for led in unearned:
            led.value(0)
        time.sleep_ms(off_ms)
    show_progress()

# ----- Keypad scanning -----
def scan_key():
    """Return a single key character if one is pressed, else None.
    Starts the key-press flash as soon as the press is confirmed."""
    for r_idx, rpin in enumerate(row_pins):
        rpin.init(Pin.OUT, value=0)    # drive this row LOW
        time.sleep_us(200)             # settle
        for c_idx, cpin in enumerate(col_pins):
            if cpin.value() == 0:      # pressed key pulls its column LOW
                time.sleep_ms(20)      # debounce: still pressed?
                if cpin.value() != 0:
                    continue
                key = KEYS[r_idx][c_idx]
                start_keypress_flash()
                while cpin.value() == 0:   # wait for release
                    service_flash()
                    time.sleep_ms(10)
                rpin.init(Pin.IN, Pin.PULL_UP)
                time.sleep_ms(20)      # ignore release bounce
                return key
        rpin.init(Pin.IN, Pin.PULL_UP) # release row back to idle
    return None

# ----- Main logic -----
def main():
    print("Connecting Wi-Fi...")
    if wifi_connect(WIFI_SSID, WIFI_PASSWORD):
        print("Wi-Fi connected.")
    else:
        print("Wi-Fi NOT connected; running offline.")

    mqtt = MqttWrapper(MQTT_CLIENT_ID, MQTT_BROKER, MQTT_PORT)
    if mqtt.client:
        mqtt.connect()

    # Button press handling (publish MQTT when pressed)
    button_event = {"pressed": False}

    def button_irq(pin):
        # IRQ context: keep it tiny; set a flag and handle in main loop
        if pin.value() == 0:  # pressed (active low)
            button_event["pressed"] = True

    button.irq(trigger=Pin.IRQ_FALLING, handler=button_irq)

    buffer = ""  # collect 4 digits
    show_progress()

    print("Ready.")

    while True:
        service_flash()

        # Handle button publish outside IRQ
        if button_event["pressed"]:
            button_event["pressed"] = False
            print("Button pressed -> MQTT publish")
            if mqtt.connected:
                try:
                    mqtt.publish(MQTT_TOPIC_BUTTON, b"doorbell")
                except Exception as e:
                    print("MQTT error on button:", e)

        # Keypad scan (every key press flashes the lights inside scan_key)
        k = scan_key()
        if k:
            if PUBLISH_KEYPRESSES and mqtt.connected:
                try:
                    mqtt.publish(MQTT_TOPIC_KEY, k.encode())
                except:
                    pass

            # Handle non-digit keys
            if k in DIGITS:
                buffer += k
                print("Key:", k, "buffer:", buffer)
            elif k == '*':
                # Clear
                buffer = ""
                print("Buffer cleared")
            elif k == '#':
                # optional: treat as "enter"
                pass
            else:
                # ignore A/B/C/D by default
                pass

            # When we have 4 digits, check a code
            if len(buffer) == 4:
                code = buffer
                buffer = ""

                if code in CODES and code not in achieved:
                    achieved.append(code)
                    show_progress()   # next light comes on and stays on
                    print("Correct code! %d of %d done." % (len(achieved), len(CODES)))

                    if len(achieved) == len(CODES):
                        print("All codes entered. All lights on, relay ON until power reset.")
                        relay_on()
                        # Relay and lights stay on from here. Power-cycle or reset the Pico to start over.
                elif code in achieved:
                    print("Code already entered previously.")
                    blink_unearned()
                else:
                    print("Incorrect code")
                    blink_unearned()

        # idle a tad
        time.sleep_ms(10)

# Run
try:
    main()
except KeyboardInterrupt:
    set_all_leds(False)
    relay_off()
    print("Stopped.")
