"""
Smart Veggie Field - MQTT Bridge
Reads serial, publishes to public broker -> GitHub Pages dashboard
"""
import json, time, threading
import paho.mqtt.client as mqtt
import serial

BROKER = "broker.emqx.io"
PORT = 1883
TOPIC_DATA = "vegfield/vegfield_001/data"
TOPIC_EVENT = "vegfield/vegfield_001/event"
TOPIC_CMD = "vegfield/vegfield_001/cmd"

latest = {}
ser_cmd = None

def on_connect(client, userdata, flags, rc):
    print(f"[MQTT] Connected (rc={rc})")
    client.subscribe(TOPIC_CMD)
    print(f"[MQTT] Subscribed to {TOPIC_CMD}")

def on_message(client, userdata, msg):
    global ser_cmd
    try:
        payload = json.loads(msg.payload.decode())
        cmd = payload.get("cmd", "")
        xlat = {"pump_on":"pump1","pump_off":"pump0","led_on":"lite1","led_off":"lite0",
                "fan_on":"fan1","fan_off":"fan0","shade_on":"shad1","shade_off":"shad0",
                "buzz_on":"buzz1","buzz_off":"buzz0","auto_mode":"auto1","manual_mode":"manu"}
        mcucmd = xlat.get(cmd, cmd)
        if ser_cmd and ser_cmd.is_open:
            ser_cmd.write((mcucmd+'\n').encode())
            print(f"[CMD] {cmd} -> {mcucmd}")
    except: pass

def serial_reader():
    global latest, ser_cmd
    while True:
        try:
            ser = serial.Serial('COM7', 115200, timeout=2)
            ser_cmd = ser
            print(f"[Serial] COM7")
            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line.startswith('{') and line.endswith('}'):
                    try:
                        data = json.loads(line)
                        if "lux" in data: latest = data
                    except: pass
        except Exception as e:
            print(f"[Serial] {e}, retry...")
            time.sleep(3)

def mqtt_loop():
    global latest
    while True:
        time.sleep(2)
        if not latest: continue

        soil_raw = latest.get("soil", 4095)
        soil_pct = max(0, min(100, 100 - soil_raw / 40.95))
        temp = latest.get("temp", 0)
        hum = latest.get("hum", 0)
        lux = latest.get("lux", 0)

        msg = {
            "data": {
                "soil": [soil_pct, soil_pct],
                "soil_temp": None,
                "temp": temp, "humi": hum, "lux": lux,
                "rain": False,
                "npk": {"valid": False, "n": 0, "p": 0, "k": 0}
            },
            "ctrl": {
                "pump": bool(latest.get("pump", 0)),
                "led": bool(latest.get("light", 0)),
                "fan": bool(latest.get("fan", 0)),
                "shade": bool(latest.get("shade", 0)),
                "buzz": bool(latest.get("buzz", 0)),
                "mode": bool(latest.get("mode", 1))
            }
        }
        try:
            client.publish(TOPIC_DATA, json.dumps(msg))
            print(f"[MQTT] {temp}C {hum}% {lux}lux soil={soil_pct:.0f}%")
        except Exception as e:
            print(f"[MQTT] Error: {e}")

if __name__ == '__main__':
    print("=== MQTT Bridge ===")
    client = mqtt.Client(client_id="vegfield_py")
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(BROKER, PORT, 60)
    client.loop_start()
    threading.Thread(target=serial_reader, daemon=True).start()
    threading.Thread(target=mqtt_loop, daemon=True).start()
    print(f"Pushing to {BROKER}")
    print("Dashboard: https://bk201-l.github.io/smart-vegetable-field/")
    try:
        while True: time.sleep(1)
    except KeyboardInterrupt:
        client.loop_stop()
        client.disconnect()
