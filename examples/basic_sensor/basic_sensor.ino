#include <IoTX.h>
#include <DHT.h>

#define DHT_PIN 32

DHT dht(DHT_PIN, DHT11);

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
