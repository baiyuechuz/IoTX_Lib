# IoTX Lib

Low-code Arduino library for ESP32 — connect to Firebase and the IoTX Dashboard with minimal code.

## Installation

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/baiyuechuz/IoTX_Lib.git
    adafruit/DHT sensor library  ; if using DHT sensor
```

Or copy the `src/` folder into your project's `lib/IoTX/` directory.

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

## API Reference

### `IoTX.begin(config)`

Initialize WiFi and Firebase. Blocks until WiFi connects (15s timeout).

```cpp
IoTX.begin({
    .wifiSSID     = "SSID",
    .wifiPassword = "PASS",
    .firebaseHost = "xxx.firebasedatabase.app",
    .firebaseAuth = "SECRET"
});
```

### `IoTXSensor`

Read/write numeric values. Maps to Temperature, Humidity, Smoke, Progress, and Chart widgets.

```cpp
IoTXSensor temp("/sensors/temperature");
temp.write(25.5);       // push to dashboard
float val = temp.read(); // read back
```

### `IoTXSwitch`

Boolean controls. Maps to Switch and LED widgets.

```cpp
IoTXSwitch led("/controls/led");
led.attachPin(2);        // auto-sync to GPIO 2
led.sync();              // read from Firebase → update pin
led.write(true);         // push state to dashboard
bool state = led.read(); // read state
```

Use `attachPin(pin, true)` for active-low relays.

### `IoTXSlider`

Numeric range controls. Maps to Range Slider widget.

```cpp
IoTXSlider slider("/controls/range-slider");
slider.attachPin(25, 0, 100);  // PWM output, range 0–100
slider.sync();                  // read from Firebase → update PWM
slider.write(50.0);            // push value
float val = slider.read();     // read value
```

### `IoTXDisplay`

Text display. Maps to LCD Text widget.

```cpp
IoTXDisplay lcd("/display/lcd-text");
lcd.write("Hello!");
lcd.printf("Temp: %.1f C", 25.3);
String text = lcd.read();
```

### `IoTXHardwareMonitor`

System metrics. Maps to HW Manager widget.

```cpp
IoTXHardwareMonitor hw("/hardware");
hw.writeCPU(45.2);
hw.writeMemory(62.0);
hw.writeDisk(80.5);
hw.writeAll(45.2, 62.0, 80.5);
```

### Utility Methods

```cpp
IoTX.isConnected();     // WiFi + Firebase ready
IoTX.isWiFiConnected();
IoTX.isFirebaseReady();
IoTX.reconnectWiFi();   // auto-reconnect
IoTX.printStatus();     // print debug info
IoTX.lock();            // mutex for advanced usage
IoTX.unlock();
```

## Firebase Path ↔ Dashboard Widget Mapping

| Firebase Path           | Type    | Dashboard Widget  |
|-------------------------|---------|-------------------|
| `/sensors/temperature`  | number  | Temperature       |
| `/sensors/humidity`     | number  | Humidity          |
| `/sensors/smoke`        | number  | Smoke             |
| `/sensors/progress`     | number  | Progress Bar      |
| `/sensors/data`         | number  | Chart             |
| `/controls/switch`      | boolean | Switch            |
| `/controls/led`         | boolean | LED               |
| `/controls/range-slider`| number  | Range Slider      |
| `/display/lcd-text`     | string  | LCD Text          |
| `/hardware/cpu`         | number  | HW Manager (CPU)  |
| `/hardware/memory`      | number  | HW Manager (RAM)  |
| `/hardware/disk`        | number  | HW Manager (Disk) |

> Paths are configurable — pass any path to match your dashboard widget's `firebasePath` setting.

## Thread Safety (FreeRTOS)

All read/write operations are mutex-protected. Safe to use across multiple FreeRTOS tasks:

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

void setup() {
    IoTX.begin({...});
    xTaskCreatePinnedToCore(taskSensors,  "S", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskControls, "C", 4096, NULL, 1, NULL, 1);
}
```

## License

MIT
