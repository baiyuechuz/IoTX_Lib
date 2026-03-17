# API Reference

## Table of Contents

- [IoTX (Main Singleton)](#iotx-main-singleton)
- [IoTXConfig](#iotxconfig)
- [IoTXSensor](#iotxsensor)
- [IoTXSwitch](#iotxswitch)
- [IoTXSlider](#iotxslider)
- [IoTXButton](#iotxbutton)
- [IoTXDisplay](#iotxdisplay)
- [IoTXHardwareMonitor](#iotxhardwaremonitor)

---

## IoTX (Main Singleton)

Global singleton that handles WiFi, Firebase, and a pool of FreeRTOS mutex-protected `FirebaseData` instances. Access it directly as `IoTX`.

### `IoTX.begin(config)` → `bool`

Initialize WiFi and Firebase connection. Blocks until WiFi connects or times out (15 seconds).

```cpp
bool ok = IoTX.begin({
    .wifiSSID     = "SSID",
    .wifiPassword = "PASS",
    .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
    .firebaseAuth = "DATABASE_SECRET"
});
```

**Returns:** `true` on success, `false` if WiFi timeout or mutex creation failed.

---

### `IoTX.isConnected()` → `bool`

Returns `true` if WiFi is connected **and** Firebase is initialized.

---

### `IoTX.isWiFiConnected()` → `bool`

Returns `true` if WiFi is connected.

---

### `IoTX.isFirebaseReady()` → `bool`

Returns `true` if `begin()` completed successfully (Firebase initialized).

---

### `IoTX.reconnectWiFi()` → `void`

Check WiFi status and reconnect if disconnected. Timeout: 10 seconds. Safe to call frequently — does nothing if already connected.

```cpp
void loop() {
    IoTX.reconnectWiFi();
    vTaskDelay(pdMS_TO_TICKS(10000));
}
```

---

### `IoTX.printStatus()` → `void`

Print connection info to Serial:

```
=== IoTX Status ===
  WiFi: Connected
  IP:   192.168.1.42
  Firebase: Ready
===================
```

---

### `IoTX.getFirebaseData()` → `FirebaseData&`

Returns the first `FirebaseData` object from the pool. For simple single-threaded use.

---

### `IoTX.acquireFirebaseData(timeoutMs)` → `int`

Acquire a `FirebaseData` object from the pool (3 instances). Returns the pool index, or `-1` on timeout.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `timeoutMs` | `uint32_t` | `1000` | Timeout in milliseconds |

---

### `IoTX.releaseFirebaseData(index)` → `void`

Release a `FirebaseData` object back to the pool.

| Parameter | Type | Description |
|-----------|------|-------------|
| `index` | `int` | Pool index returned by `acquireFirebaseData()` |

### Direct Firebase Access Example

> **Note:** `_fbdoPool` is private — only IoTX built-in classes can access it directly. For custom Firebase operations, use `getFirebaseData()`:

```cpp
// Simple (single-threaded):
FirebaseData& fbdo = IoTX.getFirebaseData();
Firebase.setString(fbdo, "/status/device", "online");
```

---

## IoTXConfig

Configuration struct passed to `IoTX.begin()`.

| Field | Type | Description |
|-------|------|-------------|
| `wifiSSID` | `const char*` | WiFi network name |
| `wifiPassword` | `const char*` | WiFi password |
| `firebaseHost` | `const char*` | Firebase RTDB host (without `https://`) |
| `firebaseAuth` | `const char*` | Database secret or legacy token |

```cpp
IoTXConfig config = {
    .wifiSSID     = "MyWiFi",
    .wifiPassword = "password",
    .firebaseHost = "my-project-default-rtdb.firebasedatabase.app",
    .firebaseAuth = "abc123secret"
};
IoTX.begin(config);
```

---

## IoTXSensor

Read/write **float values**. Use for any numeric sensor data.

**Dashboard widgets:** Temperature, Humidity, Smoke, Progress Bar, Chart

### Constructor

```cpp
IoTXSensor sensor("/sensors/temperature");
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `firebasePath` | `const char*` | Firebase RTDB path |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `write(float value)` | `bool` | Write float to Firebase. Returns `true` on success. |
| `read()` | `float` | Read float from Firebase. Returns `0` on failure. |
| `getPath()` | `const char*` | Get the Firebase path. |

### Example

```cpp
IoTXSensor temp("/sensors/temperature");
IoTXSensor hum("/sensors/humidity");

temp.write(25.5);        // push value
float val = temp.read(); // read back
```

---

## IoTXSwitch

Read/write **boolean values** (`true`/`false`). Supports GPIO pin binding for automatic hardware sync.

**Dashboard widgets:** Switch, LED

### Constructor

```cpp
IoTXSwitch mySwitch("/controls/switch");
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `firebasePath` | `const char*` | Firebase RTDB path |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `read()` | `bool` | Read boolean from Firebase. Returns last known state on failure. |
| `write(bool state)` | `bool` | Write boolean to Firebase. Returns `true` on success. |
| `attachPin(int pin, bool activeLow = false)` | `void` | Bind a GPIO pin (set as OUTPUT). |
| `sync()` | `void` | Read from Firebase and update the attached pin. |
| `getState()` | `bool` | Get last known state without Firebase call. |
| `getPath()` | `const char*` | Get the Firebase path. |

### `attachPin` Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `pin` | `int` | — | GPIO pin number |
| `activeLow` | `bool` | `false` | `true` for active-low relays (LOW = ON) |

### Example

```cpp
IoTXSwitch led("/controls/led");
led.attachPin(2);          // GPIO 2

// In a loop or task:
led.sync();                // reads Firebase → updates GPIO

// Or manual control:
led.write(true);           // turn on
bool on = led.read();      // check state
```

### Active-Low (Relay)

```cpp
IoTXSwitch relay("/controls/switch");
relay.attachPin(4, true);  // LOW = ON, HIGH = OFF
relay.sync();
```

---

## IoTXSlider

Read/write **float values** with optional PWM output via ESP32 LEDC.

**Dashboard widgets:** Slider, Range Slider

### Constructor

```cpp
IoTXSlider slider("/controls/range-slider");
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `firebasePath` | `const char*` | Firebase RTDB path |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `read()` | `float` | Read float from Firebase. Returns `0` on failure. |
| `write(float value)` | `bool` | Write float to Firebase. Returns `true` on success. |
| `attachPin(int pin, float minVal = 0, float maxVal = 100)` | `void` | Bind GPIO for PWM output (LEDC 5kHz, 8-bit). |
| `sync()` | `void` | Read from Firebase and update PWM duty cycle. |
| `getPath()` | `const char*` | Get the Firebase path. |

### `attachPin` Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `pin` | `int` | — | GPIO pin number |
| `minVal` | `float` | `0` | Minimum value (maps to 0% duty) |
| `maxVal` | `float` | `100` | Maximum value (maps to 100% duty) |

### PWM Pin Compatibility

ESP32 LEDC-capable pins: **2, 4, 12–15, 25–27, 32–33**

### Example

```cpp
IoTXSlider brightness("/controls/range-slider");
brightness.attachPin(25);          // 0–100 → 0–255 PWM
brightness.attachPin(25, 0, 1023); // custom range

// In a loop or task:
brightness.sync();                 // Firebase value → PWM
```

---

## IoTXButton

Read/write **boolean values** with toggle support. Use for momentary button actions from the dashboard.

**Dashboard widgets:** Button

### Constructor

```cpp
IoTXButton btn("/controls/button");
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `firebasePath` | `const char*` | Firebase RTDB path |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `read()` | `bool` | Read boolean from Firebase. Returns last known state on failure. |
| `write(bool state)` | `bool` | Write boolean to Firebase. Returns `true` on success. |
| `toggle()` | `bool` | Read current state, then write the opposite. Returns `true` on success. |
| `getState()` | `bool` | Get last known state without Firebase call. |
| `getPath()` | `const char*` | Get the Firebase path. |

### Example

```cpp
IoTXButton btn("/controls/button");

// In a loop or task:
bool pressed = btn.read();
if (pressed) {
    // handle button press
    btn.write(false);  // reset after handling
}

// Or use toggle:
btn.toggle();  // flips true ↔ false
```

---

## IoTXDisplay

Read/write **text strings**. Supports `printf`-style formatting.

**Dashboard widgets:** LCD Text

### Constructor

```cpp
IoTXDisplay lcd("/display/lcd-text");
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `firebasePath` | `const char*` | Firebase RTDB path |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `write(const char* text)` | `bool` | Write C string to Firebase. |
| `write(const String& text)` | `bool` | Write Arduino String to Firebase. |
| `printf(const char* format, ...)` | `bool` | Write formatted text (256 byte buffer). |
| `read()` | `String` | Read text from Firebase. Returns empty on failure. |
| `getPath()` | `const char*` | Get the Firebase path. |

### Example

```cpp
IoTXDisplay lcd("/display/lcd-text");

lcd.write("Hello World!");
lcd.write(String("Dynamic"));
lcd.printf("Temp: %.1f C", 25.3);
lcd.printf("Uptime: %lu s", millis() / 1000);

String text = lcd.read();
```

---

## IoTXHardwareMonitor

Write **system metrics** as percentages (0–100). Writes to `{basePath}/cpu`, `{basePath}/memory`, and `{basePath}/disk`.

**Dashboard widgets:** HW Manager

### Constructor

```cpp
IoTXHardwareMonitor hw("/hardware");        // default
IoTXHardwareMonitor hw("/my/custom/path");  // custom base path
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `basePath` | `const char*` | `"/hardware"` | Base path in Firebase |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `writeCPU(float percent)` | `bool` | Write to `{basePath}/cpu` |
| `writeMemory(float percent)` | `bool` | Write to `{basePath}/memory` |
| `writeDisk(float percent)` | `bool` | Write to `{basePath}/disk` |
| `writeAll(float cpu, float memory, float disk)` | `bool` | Write all three. `true` only if all succeed. |
| `getPath()` | `const char*` | Get the base path. |

### Example

```cpp
IoTXHardwareMonitor hw;  // defaults to "/hardware"

hw.writeCPU(45.2);
hw.writeMemory(62.0);
hw.writeDisk(80.5);

// Or all at once:
hw.writeAll(45.2, 62.0, 80.5);

// ESP32 memory usage:
float mem = 100.0 - ((float)ESP.getFreeHeap() / ESP.getHeapSize() * 100.0);
hw.writeMemory(mem);
```
