"""
Smart Veggie Field - Serial Bridge with Command Support
"""
import json, http.server, threading, time

latest = {"lux":0,"temp":0,"hum":0,"soil":4095,"rain":4095,"pump":0,"fan":0,"light":0}
ser = None

def serial_reader():
    global latest, ser
    import serial
    while True:
        try:
            ser = serial.Serial('COM7', 115200, timeout=2)
            print(f"[Serial] {ser.port} @ {ser.baudrate}")
            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line.startswith('{') and line.endswith('}'):
                    try:
                        data = json.loads(line)
                        if "lux" in data: latest.update(data)
                    except: pass
        except Exception as e:
            print(f"[Serial] {e}, retry 3s...")
            time.sleep(3)

class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/api/sensors':
            s = latest["soil"]; s_pct = max(0, min(100, 100 - s/40.95))
            self._json({
                "data": {
                    "soil": [s_pct, s_pct], "temp": latest["temp"],
                    "humi": latest["hum"], "lux": latest["lux"],
                    "rain": latest["rain"] < 500,
                    "soil_temp": None,
                    "npk": {"valid":False,"n":0,"p":0,"k":0}
                },
                "ctrl": {
                    "pump": bool(latest["pump"]), "fan": bool(latest["fan"]),
                    "led": bool(latest["light"]), "shade": bool(latest.get("shade",0)),
                    "buzz": bool(latest.get("buzz",0)),
                    "mode": bool(latest.get("mode",1))
                },
                "alarm": {
                    "active": latest.get("soil",4095) > 3000 or latest.get("temp",0) > 28,
                    "soil_dry": latest.get("soil",4095) > 3000,
                    "temp_high": latest.get("temp",0) > 28
                }
            })
        elif self.path == '/' or self.path.endswith('.html'):
            self._html(open('cloud/dashboard_http.html','r',encoding='utf-8').read())
        else:
            self.send_response(404); self.end_headers()

    def do_POST(self):
        if self.path == '/api/sensors':
            length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(length)
            try:
                data = json.loads(body)
                cmd = data.get('cmd','')
                # translate to MCU format: pump_on->pump=1, pump_off->pump=0 etc
                xlat = {"pump_on":"pump1","pump_off":"pump0",
                        "led_on":"lite1","led_off":"lite0",
                        "fan_on":"fan1","fan_off":"fan0",
                        "shade_on":"shad1","shade_off":"shad0",
                        "buzz_on":"buzz1","buzz_off":"buzz0",
                        "auto_mode":"auto1","manual_mode":"manu"}
                mcucmd = xlat.get(cmd, cmd)
                if ser and ser.is_open:
                    ser.write((mcucmd+'\n').encode())
                    print(f"[CMD] {cmd} -> {mcucmd}")
                self._json({"ok":True})
            except: self._json({"ok":False})
        else:
            self.send_response(404); self.end_headers()

    def _json(self, data):
        self.send_response(200)
        self.send_header('Content-Type','application/json')
        self.send_header('Access-Control-Allow-Origin','*')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())

    def _html(self, content):
        self.send_response(200)
        self.send_header('Content-Type','text/html; charset=utf-8')
        self.end_headers()
        self.wfile.write(content.encode('utf-8'))

    def log_message(self, fmt, *args): pass

if __name__ == '__main__':
    print("=== Smart Veggie Field Bridge ===")
    threading.Thread(target=serial_reader, daemon=True).start()
    print("HTTP: http://localhost:8080")
    http.server.HTTPServer(('0.0.0.0', 8080), Handler).serve_forever()
