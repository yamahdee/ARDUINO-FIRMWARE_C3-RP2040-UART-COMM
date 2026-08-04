from machine import UART, Pin
import time
import ujson

# Initialize UART on Pico
uart = UART(
    0,
    baudrate=115200,
    tx=Pin(0),
    rx=Pin(1)
)

def send_json_cmd(payload):
    """Converts a dictionary to JSON, sends it with a newline character,

    and handles the structured response.
    """
    try:
        # Convert dictionary to string and add the essential newline
        packet = ujson.dumps(payload) + "\n"
        uart.write(packet)
        print(f"\n[SENT TO ESP]: {packet.strip()}")
    except Exception as e:
        print(f"Error encoding JSON packet: {e}")
        return

    # Wait up to 16 seconds for a response (crucial for 15s WiFi timeout loops)
    start_time = time.ticks_ms()
    response_received = False
    
    while time.ticks_diff(time.ticks_ms(), start_time) < 16000:
        if uart.any():
            raw_response = uart.readline()
            if raw_response:
                try:
                    # Clean and decode incoming response data
                    clean_text = raw_response.decode('utf-8').strip()
                    
                    # Convert response back into a Python Dictionary
                    response_data = ujson.loads(clean_text)
                    
                    # Extract variables directly by key name
                    status = response_data.get("status", "UNKNOWN")
                    msg = response_data.get("msg", "")
                    
                    print(f"[ESP RESPONSE] Status: {status} | Msg: {msg}")
                    response_received = True
                    break
                except Exception as e:
                    print(f"Raw UART Text received (Failed to parse JSON): {raw_response}")
                    response_received = True
                    break
        time.sleep_ms(50)

    if not response_received:
        print("[TIMEOUT] No response received from ESP32-C3 within 16 seconds.")

# --- INTERACTIVE CONTROL LOOP ---
print("ESP32-C3 Controller Ready!")
print("Commands: ping, status, led_on, led_off, connect, ap, ap_sta, disconnect, off")

while True:
    user_input = input("PICO> ").strip().lower()

    if user_input == "ping":
        send_json_cmd({"cmd": "PING"})
        
    elif user_input == "status":
        send_json_cmd({"cmd": "STATUS"})
        
    elif user_input == "led_on":
        send_json_cmd({"cmd": "LED", "action": "ON"})
        
    elif user_input == "led_off":
        send_json_cmd({"cmd": "LED", "action": "OFF"})
        
    elif user_input == "connect":
        # Leave inputs blank to test the ESP's built-in default credentials fallback
        ssid = input("Router SSID (Enter for default): ")
        password = input("Router Password (Enter for default): ")
        
        payload = {"cmd": "WIFI", "action": "CONNECT"}
        if ssid: payload["ssid"] = ssid
        if password: payload["pass"] = password
        send_json_cmd(payload)
        
    elif user_input == "ap":
        ap_ssid = input("New AP SSID (Enter for default): ")
        ap_pass = input("New AP Password (Enter for default): ")
        
        payload = {"cmd": "WIFI", "action": "START_AP"}
        if ap_ssid: payload["ssid"] = ap_ssid
        if ap_pass: payload["pass"] = ap_pass
        send_json_cmd(payload)
        
    elif user_input == "ap_sta":
        ap_ssid = input("AP SSID (Enter for default): ")
        ap_pass = input("AP Password (Enter for default): ")
        sta_ssid = input("Router SSID (Enter for default): ")
        sta_pass = input("Router Password (Enter for default): ")
        
        payload = {"cmd": "WIFI", "action": "START_AP_STA"}
        if ap_ssid: payload["ap_ssid"] = ap_ssid
        if ap_pass: payload["ap_pass"] = ap_pass
        if sta_ssid: payload["ssid"] = sta_ssid
        if sta_pass: payload["pass"] = sta_pass
        send_json_cmd(payload)
        
    elif user_input == "disconnect":
        send_json_cmd({"cmd": "WIFI", "action": "DISCONNECT"})
        
    elif user_input == "wifi_status":
        send_json_cmd({"cmd": "WIFI", "action": "STATUS"})
        
    elif user_input == "off":
        send_json_cmd({"cmd": "WIFI", "action": "OFF"})
        
    else:
        print("Unknown command shortcut.")

    time.sleep(1)
