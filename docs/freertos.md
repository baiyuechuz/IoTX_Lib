# FreeRTOS Guide

## Thread Safety

All IoTX `read()`, `write()`, and `sync()` methods are **mutex-protected**. You can safely use IoTX objects across multiple FreeRTOS tasks without any additional locking.

```cpp
IoTXSensor temperature("/sensors/temperature");
IoTXSwitch led("/controls/led");

// Task 1 — reads sensor, writes to Firebase
void taskSensors(void* p) {
    while (true) {
        temperature.write(dht.readTemperature());  // safe
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Task 2 — reads Firebase, updates GPIO
void taskControls(void* p) {
    led.attachPin(2);
    while (true) {
        led.sync();  // safe
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

## Creating Tasks

Use `xTaskCreatePinnedToCore` to create tasks on a specific core:

```cpp
void setup() {
    IoTX.begin({...});

    xTaskCreatePinnedToCore(
        taskSensors,   // function
        "Sensors",     // name (for debugging)
        4096,          // stack size (bytes)
        NULL,          // parameter
        1,             // priority (1 = low, higher = more priority)
        NULL,          // task handle (optional)
        1              // core (0 or 1)
    );

    xTaskCreatePinnedToCore(taskControls, "Controls", 4096, NULL, 1, NULL, 1);
}

void loop() {
    IoTX.reconnectWiFi();
    vTaskDelay(pdMS_TO_TICKS(10000));
}
```

### Parameter Guide

| Parameter | Recommended | Notes |
|-----------|-------------|-------|
| Stack size | `4096` | Increase to `8192` if you get crashes |
| Priority | `1` | Use `2` for time-critical tasks |
| Core | `1` | Core 0 runs WiFi/BT, core 1 is for user tasks |

## Recommended Task Structure

### Sensor Task (push data)

```cpp
void taskSensors(void* p) {
    dht.begin();  // init hardware inside the task

    while (true) {
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t)) temperature.write(t);
        if (!isnan(h)) humidity.write(h);

        vTaskDelay(pdMS_TO_TICKS(5000));  // every 5 seconds
    }
}
```

### Control Task (read dashboard → update hardware)

```cpp
void taskControls(void* p) {
    led.attachPin(2);        // init pins inside the task
    slider.attachPin(25);

    while (true) {
        led.sync();
        slider.sync();

        vTaskDelay(pdMS_TO_TICKS(500));  // every 500ms for responsive controls
    }
}
```

### WiFi Watchdog (in `loop()` or separate task)

```cpp
void loop() {
    IoTX.reconnectWiFi();
    vTaskDelay(pdMS_TO_TICKS(10000));  // check every 10 seconds
}
```

## Timing Guidelines

| Operation | Recommended Interval | Reason |
|-----------|---------------------|--------|
| Sensor reads | 2000–10000 ms | Avoid flooding Firebase |
| Control sync | 200–500 ms | Responsive user controls |
| WiFi check | 10000–30000 ms | Low overhead |
| Display update | 1000–5000 ms | Readable update rate |

## Manual Mutex (Advanced)

If you need to perform multiple Firebase operations atomically:

```cpp
if (IoTX.lock(2000)) {  // 2 second timeout
    FirebaseData& fbdo = IoTX.getFirebaseData();
    Firebase.setFloat(fbdo, "/batch/val1", 1.0);
    Firebase.setFloat(fbdo, "/batch/val2", 2.0);
    Firebase.setFloat(fbdo, "/batch/val3", 3.0);
    IoTX.unlock();
}
```

> ⚠️ Always call `unlock()` after `lock()`. Never hold the mutex for long periods — it blocks all other IoTX operations.

## Common Pitfalls

### Stack overflow crash

```
Guru Meditation Error: Core 1 panic'ed (Unhandled debug exception)
```

**Fix:** Increase stack size from `4096` to `8192`.

### Task not running

Make sure `IoTX.begin()` is called **before** creating tasks — the mutex must exist first.

### `loop()` blocking

If you use `delay()` in `loop()`, use `vTaskDelay(pdMS_TO_TICKS(ms))` instead so other tasks can run:

```cpp
void loop() {
    IoTX.reconnectWiFi();
    vTaskDelay(pdMS_TO_TICKS(10000));  // yields to other tasks
}
```
