# Examples

## Basic Sensor

Read DHT11 temperature and humidity, push to dashboard every 5 seconds.

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
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.asia-southeast1.firebasedatabase.app",
        .firebaseAuth = "YOUR_DATABASE_SECRET"
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

## Full Dashboard

All widget types with FreeRTOS tasks.

```cpp
#include <IoTX.h>
#include <DHT.h>

#define DHT_PIN    32
#define LED_PIN    2
#define SLIDER_PIN 25
#define SMOKE_PIN  34

DHT dht(DHT_PIN, DHT11);

// Sensors
IoTXSensor temperature("/sensors/temperature");
IoTXSensor humidity("/sensors/humidity");
IoTXSensor smoke("/sensors/smoke");

// Controls
IoTXSwitch led("/controls/led");
IoTXSlider slider("/controls/range-slider");
IoTXButton btn("/controls/button");

// Display
IoTXDisplay lcd("/display/lcd-text");

// Hardware Monitor
IoTXHardwareMonitor hw("/hardware");

void taskSensors(void* pvParameters) {
    dht.begin();
    while (true) {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t)) temperature.write(t);
        if (!isnan(h)) humidity.write(h);

        smoke.write(analogRead(SMOKE_PIN) / 40.95);

        float memPercent = 100.0 - ((float)ESP.getFreeHeap() / ESP.getHeapSize() * 100.0);
        hw.writeMemory(memPercent);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void taskControls(void* pvParameters) {
    led.attachPin(LED_PIN);
    slider.attachPin(SLIDER_PIN);
    while (true) {
        led.sync();
        slider.sync();
        if (btn.read()) {
            Serial.println("Button pressed!");
            btn.write(false);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void setup() {
    Serial.begin(115200);

    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.asia-southeast1.firebasedatabase.app",
        .firebaseAuth = "YOUR_DATABASE_SECRET"
    });

    lcd.write("IoTX Dashboard Ready!");
    IoTX.printStatus();

    xTaskCreatePinnedToCore(taskSensors,  "Sensors",  4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskControls, "Controls", 4096, NULL, 1, NULL, 1);
}

void loop() {
    IoTX.reconnectWiFi();
    vTaskDelay(pdMS_TO_TICKS(10000));
}
```

## Active-Low Relay

Many relay modules are active-low (LOW = ON, HIGH = OFF):

```cpp
#include <IoTX.h>

IoTXSwitch relay("/controls/switch");

void setup() {
    Serial.begin(115200);
    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });

    relay.attachPin(4, true);  // active-low: true → LOW, false → HIGH
}

void loop() {
    relay.sync();
    delay(500);
}
```

## Multiple LEDs (Array)

Control multiple LEDs with an array:

```cpp
#include <IoTX.h>

constexpr int NUM_LEDS = 4;
constexpr int LED_PINS[NUM_LEDS] = {12, 14, 27, 26};

IoTXSwitch leds[NUM_LEDS] = {
    IoTXSwitch("/controls/led1"),
    IoTXSwitch("/controls/led2"),
    IoTXSwitch("/controls/led3"),
    IoTXSwitch("/controls/led4"),
};

void setup() {
    Serial.begin(115200);
    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].attachPin(LED_PINS[i]);
    }
}

void loop() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].sync();
    }
    delay(500);
}
```

## Button Control

Respond to button presses from the dashboard:

```cpp
#include <IoTX.h>

IoTXButton btn("/controls/button");

void setup() {
    Serial.begin(115200);
    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });
}

void loop() {
    if (btn.read()) {
        Serial.println("Button pressed!");
        // Do something...
        btn.write(false);  // reset button state
    }
    delay(500);
}
```

### Toggle Mode

Use `toggle()` to flip the state each time:

```cpp
IoTXButton toggle("/controls/button");

void loop() {
    // Read the current state
    bool state = toggle.getState();
    Serial.printf("Button state: %s\n", state ? "ON" : "OFF");
    delay(1000);
}
```

## LCD Display with Live Data

Update the LCD Text widget with formatted sensor data:

```cpp
#include <IoTX.h>
#include <DHT.h>

DHT dht(32, DHT11);
IoTXDisplay lcd("/display/lcd-text");

void setup() {
    Serial.begin(115200);
    dht.begin();
    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });
}

void loop() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        lcd.printf("T:%.1fC H:%.0f%%", t, h);
    }

    delay(5000);
}
```

## Slider-Controlled LED Brightness

Use a dashboard slider to control LED brightness via PWM:

```cpp
#include <IoTX.h>

IoTXSlider brightness("/controls/range-slider");

void setup() {
    Serial.begin(115200);
    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });

    brightness.attachPin(25, 0, 100);  // GPIO 25, range 0–100
}

void loop() {
    brightness.sync();  // dashboard slider → PWM duty cycle
    delay(200);
}
```

## ESP32 Memory Monitor

Report ESP32 heap usage to the HW Manager widget:

```cpp
#include <IoTX.h>

IoTXHardwareMonitor hw("/hardware");

void setup() {
    Serial.begin(115200);
    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });
}

void loop() {
    float totalHeap = ESP.getHeapSize();
    float freeHeap = ESP.getFreeHeap();
    float memPercent = 100.0 - (freeHeap / totalHeap * 100.0);

    hw.writeMemory(memPercent);

    delay(5000);
}
```

## Direct Firebase Access

For operations not covered by built-in classes:

```cpp
#include <IoTX.h>

void setup() {
    Serial.begin(115200);
    IoTX.begin({
        .wifiSSID     = "YOUR_WIFI",
        .wifiPassword = "YOUR_PASS",
        .firebaseHost = "xxx-default-rtdb.firebasedatabase.app",
        .firebaseAuth = "YOUR_SECRET"
    });

    // Direct Firebase write using getFirebaseData()
    FirebaseData& fbdo = IoTX.getFirebaseData();
    Firebase.setString(fbdo, "/status/device", "online");
    Firebase.setInt(fbdo, "/status/boot_count", 42);
}

void loop() {
    delay(10000);
}
```
