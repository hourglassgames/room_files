from machine import Pin
import time

# GPIO list: GP0-22 and GP26
GPIO_PINS = list(range(0, 23)) + [26]

# Set ACTIVE_STATE to 1 for active-high relays, or 0 for active-low relays.
ACTIVE_STATE = 1
INACTIVE_STATE = 0 if ACTIVE_STATE == 1 else 1

# Initialize relay pins
relays = [Pin(gp, Pin.OUT) for gp in GPIO_PINS]

# Ensure all relays are off at start
for r in relays:
    r.value(INACTIVE_STATE)

time.sleep(1.0)

while True:
    for idx, r in enumerate(relays):
        # Turn relay on
        r.value(ACTIVE_STATE)
        time.sleep(1.0)

        # Turn relay off
        r.value(INACTIVE_STATE)
        time.sleep(0.2)

    # Optional pause between full cycles
    time.sleep(1.0)
