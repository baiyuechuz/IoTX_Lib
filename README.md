# IoTX Lib

Low-code Arduino library for ESP32 — connect to Firebase and the [IoTX Dashboard](https://github.com/baiyuechuz/IoTX_Website) with minimal code.

## Installation

```ini
# platformio.ini
lib_deps =
    baiyuechu/IoTX
```

## Quick Start

```cpp
#include <IoTX.h>
#include <DHT.h>

DHT dht(32, DHT11);
IoTXSensor temperature("/sensors/temperature");
IoTXSensor humidity("/sensors/humidity");

void setup() {
    Serial.begin(115200);
    dht.begin();
    IoTX.begin({
        .wifiSSID     = "MyWiFi",
        .wifiPassword = "password",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });
}

void loop() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature.write(t);
    if (!isnan(h)) humidity.write(h);
    delay(5000);
}
```

## Classes

| Class | Use for | Example |
|---|---|---|
| `IoTXSensor` | Numeric values (temperature, humidity, smoke…) | `sensor.write(25.5)` |
| `IoTXSwitch` | Boolean on/off with GPIO binding | `sw.attachPin(2); sw.sync()` |
| `IoTXSlider` | Range values with PWM output | `slider.attachPin(25); slider.sync()` |
| `IoTXDisplay` | Text with printf support | `lcd.printf("T:%.1fC", t)` |
| `IoTXHardwareMonitor` | CPU/Memory/Disk metrics | `hw.writeAll(45, 62, 80)` |

## Firebase Path ↔ Dashboard Widget

| Path | Type | Widget |
|---|---|---|
| `/sensors/temperature` | float | Temperature |
| `/sensors/humidity` | float | Humidity |
| `/sensors/smoke` | float | Smoke |
| `/sensors/progress` | float | Progress Bar |
| `/sensors/data` | float | Chart |
| `/controls/switch` | bool | Switch |
| `/controls/led` | bool | LED |
| `/controls/range-slider` | float | Range Slider |
| `/display/lcd-text` | string | LCD Text |
| `/hardware/cpu` | float | HW Manager |
| `/hardware/memory` | float | HW Manager |
| `/hardware/disk` | float | HW Manager |

> Paths are configurable — use any path that matches your widget's `firebasePath` setting.

## Thread Safety

All operations are FreeRTOS mutex-protected. Safe across multiple tasks:

```cpp
void taskSensors(void* p) {
    while (true) {
        temperature.write(dht.readTemperature());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void taskControls(void* p) {
    led.attachPin(2);
    while (true) {
        led.sync();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

## Full Documentation

See [`docs/`](docs/) for detailed guides:

- [Getting Started](docs/getting-started.md) · [API Reference](docs/api-reference.md) · [Firebase Paths](docs/firebase-paths.md) · [Examples](docs/examples.md) · [FreeRTOS](docs/freertos.md) · [Troubleshooting](docs/troubleshooting.md)

## License

MIT
