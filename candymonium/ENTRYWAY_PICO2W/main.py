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

# 3 unique 4-digit codes. Order doesn't matter; each correct code lights its own LED.
CODES = {
    "6742": 0,   # when entered, turn on LED 0
    "9984": 1,   # when entered, turn on LED 1
    "0368": 2,   # when entered, turn on LED 2, and if all three done -> relay
}

# Relay unlock time (seconds) once all codes have been entered
RELAY_PULSE_SECONDS = 5

# ====== PIN ASSIGNMENTS (change if needed) ======
# Keypad: rows (outputs) and cols (inputs)
# Choose 8 free GPIO pins. The Pico 2 W pins below are good defaults.
KEYPAD_ROWS = [2, 3, 4, 5]   # R1..R4 as OUTPUT
KEYPAD_COLS = [6, 7, 8, 9]   # C1..C4 as INPUT with PULL_DOWN

# Three status LEDs (through resistors to GND). These go HIGH to turn on.
LED_PINS = [10, 11, 12]

# Relay control pin (to a transistor/relay module). HIGH = active (adjust if needed).
RELAY_PIN = 13

# Separate button pin that publishes an MQTT message when pressed
# Wire button to GND; use internal PULL_UP; pressed = LOW
BUTTON_PIN = 14
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
row_pins = [Pin(p, Pin.OUT, value=0) for p in KEYPAD_ROWS]
col_pins = [Pin(p, Pin.IN, Pin.PULL_DOWN) for p in KEYPAD_COLS]

leds = [Pin(p, Pin.OUT, value=0) for p in LED_PINS]
relay = Pin(RELAY_PIN, Pin.OUT, value=0)

button = Pin(BUTTON_PIN, Pin.IN, Pin.PULL_UP)

# ----- Key map (4x4) -----
# Typical layout:
# [ [ '1','2','3','A' ],
#   [ '4','5','6','B' ],
#   [ '7','8','9','C' ],
#   [ '*','0','#','D' ] ]
KEYS = [
    ['1','2','3','A'],
    ['4','5','6','B'],
    ['7','8','9','C'],
    ['*','0','#','D']
]

# We'll only accept digits into the 4-digit buffer. Other keys can be ignored or used as clear.
DIGITS = set(list("0123456789"))

# ----- Helpers -----
def set_all_leds(state):
    for l in leds:
        l.value(1 if state else 0)

def flash_all_leds(times=3, on_ms=120, off_ms=120):
    for _ in range(times):
        set_all_leds(True)
        time.sleep_ms(on_ms)
        set_all_leds(False)
        time.sleep_ms(off_ms)

def pulse_relay(seconds):
    relay.value(1)
    time.sleep(seconds)
    relay.value(0)

def indicate_code_led(idx):
    # turn on only the LED for this code index (but keep others as they were—solid means "achieved")
    leds[idx].value(1)

# ----- Keypad scanning -----
def scan_key():
    """Return a single key character if exactly one is pressed, else None."""
    # Drive each row HIGH one at a time; read columns
    key_found = None
    for r_idx, rpin in enumerate(row_pins):
        rpin.value(1)
        # small settle time
        time.sleep_us(200)
        for c_idx, cpin in enumerate(col_pins):
            if cpin.value():
                key_found = KEYS[r_idx][c_idx]
                # simple debounce: wait until release
                while cpin.value():
                    time.sleep_ms(10)
                rpin.value(0)
                return key_found
        rpin.value(0)
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

    # Track which codes have been entered
    achieved = set()  # holds code indices (0,1,2) that are complete

    # Button press handling (publish MQTT when pressed)
    button_event = {"pressed": False}

    def button_irq(pin):
        # IRQ context: keep it tiny; set a flag and handle in main loop
        if pin.value() == 0:  # pressed (active low)
            button_event["pressed"] = True

    button.irq(trigger=Pin.IRQ_FALLING, handler=button_irq)

    buffer = ""  # collect 4 digits

    print("Ready.")

    while True:
        # Handle button publish outside IRQ
        if button_event["pressed"]:
            button_event["pressed"] = False
            print("Button pressed -> MQTT publish")
            if mqtt.connected:
                try:
                    mqtt.publish(MQTT_TOPIC_BUTTON, b"doorbell")
                except Exception as e:
                    print("MQTT error on button:", e)

        # Keypad scan
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
                # Flash all LEDs while "checking"
                flash_all_leds(3, 120, 120)

                # Did it match any code?
                if buffer in CODES:
                    code_idx = CODES[buffer]
                    if code_idx not in achieved:
                        achieved.add(code_idx)
                        indicate_code_led(code_idx)
                        print("Correct code! Index:", code_idx, "Achieved:", achieved)
                    else:
                        print("Code already entered previously.")
                else:
                    print("Incorrect code")

                buffer = ""

                # If all three codes are achieved, lock open
                if len(achieved) == 3:
                    print("All codes achieved. Unlocking relay...")
                    # Ensure all LEDs solid ON
                    set_all_leds(True)
                    pulse_relay(RELAY_PULSE_SECONDS)
                    # Optionally, keep them on and stop, or reset to require codes again.
                    # Here we keep them on and continue running.
        # idle a tad
        time.sleep_ms(10)

# Run
try:
    main()
except KeyboardInterrupt:
    set_all_leds(False)
    relay.value(0)
    print("Stopped.")
