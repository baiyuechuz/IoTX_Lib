# Getting Started

## Requirements

| Component | Requirement |
|-----------|-------------|
| Board | ESP32 (any variant) |
| Framework | Arduino |
| Platform | PlatformIO (`espressif32`) |
| Firebase | Realtime Database with legacy token |

## Installation

### PlatformIO Registry (recommended)

```ini
# platformio.ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps =
    baiyuechu/IoTX
    adafruit/DHT sensor library   ; optional, for DHT sensors
```

### GitHub URL

```ini
lib_deps =
    https://github.com/baiyuechuz/IoTX_Lib.git
```

### Manual

Copy the `src/` folder into your project's `lib/IoTX/` directory.

## Firebase Setup

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Create a project (or use existing)
3. Go to **Realtime Database** → **Create Database**
4. Choose a region (e.g., `asia-southeast1`)
5. Start in **test mode** for development
6. Note your **database URL** — looks like `xxx-default-rtdb.asia-southeast1.firebasedatabase.app`
7. Go to **Project Settings** → **Service accounts** → **Database secrets** to get your legacy token

## Wiring (DHT11 example)

```
ESP32           DHT11
─────           ─────
3.3V  ────────  VCC
GND   ────────  GND
GPIO32 ───────  DATA (with 10kΩ pull-up to VCC)
```

## First Sketch

```cpp
#include <IoTX.h>
#include <DHT.h>

DHT dht(32, DHT11);
IoTXSensor temperature("/sensors/temperature");
IoTXSensor humidity("/sensors/humidity");

void setup() {
    Serial.begin(115200);
    dht.begin();

    bool ok = IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.asia-southeast1.firebasedatabase.app",
        .firebaseAuth = "YOUR_DATABASE_SECRET"
    });

    if (!ok) {
        Serial.println("IoTX init failed!");
        while (true) delay(1000);
    }

    IoTX.printStatus();
}

void loop() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) temperature.write(t);
    if (!isnan(h)) humidity.write(h);

    delay(5000);
}
```

## Verify

1. Upload the sketch
2. Open Serial Monitor at 115200 baud
3. You should see:

```
[IoTX] WiFi connecting...
[IoTX] WiFi OK: 192.168.1.42
[IoTX] Ready
=== IoTX Status ===
  WiFi: Connected
  IP:   192.168.1.42
  Firebase: Ready
===================
```

4. Check your Firebase Console → Realtime Database — you should see values under `/sensors/temperature` and `/sensors/humidity`
5. Open the IoTX Dashboard, add a **Temperature** widget with path `/sensors/temperature`, and see live data

## Next Steps

- [API Reference](api-reference.md) — Learn all available classes and methods
- [Firebase Path Mapping](firebase-paths.md) — See which paths map to which widgets
- [Examples](examples.md) — More use cases
