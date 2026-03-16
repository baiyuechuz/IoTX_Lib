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

        // Report ESP32 free heap as "memory"
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
