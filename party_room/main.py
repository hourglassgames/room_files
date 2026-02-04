import network
import socket
import time
from machine import Pin

# =======================
# USER CONFIGURATION
# =======================
WIFI_SSID = "HGG Private"
WIFI_PASSWORD = "HGGamespass10!"

# Set to 0 for active-low relay boards, or 1 for active-high relay boards.
ACTIVE_STATE = 0
INACTIVE_STATE = 0 if ACTIVE_STATE == 1 else 1

RELAY_PULSE_SECONDS = 0.5
SERVER_PORT = 80

# Relay GPIO list: GP0-22 and GP26 (24 relays total)
GPIO_PINS = list(range(0, 23)) + [26]

# =======================
# RELAY SETUP
# =======================
relays = [Pin(gp, Pin.OUT) for gp in GPIO_PINS]
for relay in relays:
    relay.value(INACTIVE_STATE)


def pulse_relay(relay_number):
    """Relay numbers are 1..24."""
    if relay_number < 1 or relay_number > len(relays):
        return False

    relay = relays[relay_number - 1]
    relay.value(ACTIVE_STATE)
    time.sleep(RELAY_PULSE_SECONDS)
    relay.value(INACTIVE_STATE)
    return True


def connect_wifi(ssid, password, timeout_seconds=20):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    if wlan.isconnected():
        return wlan

    print("Connecting to Wi-Fi...")
    wlan.connect(ssid, password)

    start = time.ticks_ms()
    while not wlan.isconnected():
        if time.ticks_diff(time.ticks_ms(), start) > timeout_seconds * 1000:
            raise RuntimeError("Wi-Fi connection timeout")
        time.sleep(0.2)

    ip_address = wlan.ifconfig()[0]
    print("Wi-Fi connected:", ip_address)
    print("IP URL: http://{}".format(ip_address))
    return wlan


def extract_relay_number(request_text):
    """Supports /relay/<n>, ?relay=<n>, body relay=<n>, or JSON {'relay': n}."""
    lines = request_text.split("\r\n")
    if not lines:
        return None

    # First line like: POST /relay/7 HTTP/1.1
    request_line = lines[0]
    parts = request_line.split(" ")
    if len(parts) < 2:
        return None

    method = parts[0]
    path = parts[1]
    if method != "POST":
        return None

    if path.startswith("/relay/"):
        try:
            return int(path.split("/relay/")[1].split("?")[0])
        except:
            return None

    if "?relay=" in path:
        try:
            return int(path.split("?relay=")[1].split("&")[0])
        except:
            return None

    # Body parsing
    body = request_text.split("\r\n\r\n", 1)[1] if "\r\n\r\n" in request_text else ""
    if "relay=" in body:
        try:
            return int(body.split("relay=")[1].split("&")[0].strip())
        except:
            return None

    if '"relay"' in body:
        digits = ""
        capture = False
        for ch in body:
            if ch.isdigit():
                capture = True
                digits += ch
            elif capture:
                break
        if digits:
            try:
                return int(digits)
            except:
                return None

    return None


def send_response(client, status, body):
    response = (
        "HTTP/1.1 {status}\r\n"
        "Content-Type: text/plain\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "Content-Length: {length}\r\n"
        "\r\n"
        "{body}"
    ).format(status=status, length=len(body), body=body)
    client.send(response.encode())


def run_server():
    addr = socket.getaddrinfo("0.0.0.0", SERVER_PORT)[0][-1]
    server = socket.socket()
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(addr)
    server.listen(1)
    print("HTTP server listening on port", SERVER_PORT)

    while True:
        client, client_addr = server.accept()
        try:
            raw_request = client.recv(2048)
            if not raw_request:
                client.close()
                continue

            request_text = raw_request.decode("utf-8", "ignore")
            if request_text.startswith("OPTIONS "):
                send_response(client, "204 No Content", "")
                continue

            relay_number = extract_relay_number(request_text)

            if relay_number is None:
                send_response(
                    client,
                    "400 Bad Request",
                    "Use POST /relay/<n> where n is 1..24",
                )
            elif pulse_relay(relay_number):
                msg = "Relay {} pulsed for {}s".format(relay_number, RELAY_PULSE_SECONDS)
                print("Request from", client_addr, "->", msg)
                send_response(client, "200 OK", msg)
            else:
                send_response(client, "404 Not Found", "Relay must be 1..24")
        except Exception as exc:
            print("Request handling error:", exc)
            try:
                send_response(client, "500 Internal Server Error", "Server error")
            except:
                pass
        finally:
            client.close()


try:
    connect_wifi(WIFI_SSID, WIFI_PASSWORD)
    run_server()
except Exception as e:
    print("Fatal error:", e)
    while True:
        time.sleep(1)
