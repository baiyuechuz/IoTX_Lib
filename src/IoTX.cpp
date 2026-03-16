#include "IoTX.h"

IoTXClass IoTX;

// ============================================================
// IoTXClass
// ============================================================

IoTXClass::IoTXClass()
    : _mutex(nullptr), _initialized(false),
      _wifiSSID(nullptr), _wifiPassword(nullptr) {}

bool IoTXClass::begin(const IoTXConfig& config) {
    _wifiSSID = config.wifiSSID;
    _wifiPassword = config.wifiPassword;

    // Connect WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifiSSID, _wifiPassword);
    Serial.print("[IoTX] WiFi connecting");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
        if (millis() - start > 15000) {
            Serial.println("\n[IoTX] WiFi connection timeout");
            return false;
        }
    }
    Serial.printf("\n[IoTX] WiFi OK: %s\n", WiFi.localIP().toString().c_str());

    // Firebase
    _config.host = config.firebaseHost;
    _config.signer.tokens.legacy_token = config.firebaseAuth;
    Firebase.begin(&_config, &_auth);
    Firebase.reconnectWiFi(true);

    // Mutex
    _mutex = xSemaphoreCreateMutex();
    if (_mutex == nullptr) {
        Serial.println("[IoTX] Mutex creation failed");
        return false;
    }

    _initialized = true;
    Serial.println("[IoTX] Ready");
    return true;
}

bool IoTXClass::isConnected() const {
    return _initialized && WiFi.status() == WL_CONNECTED;
}

bool IoTXClass::isWiFiConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool IoTXClass::isFirebaseReady() const {
    return _initialized;
}

FirebaseData& IoTXClass::getFirebaseData() {
    return _fbdo;
}

bool IoTXClass::lock(uint32_t timeoutMs) {
    if (_mutex == nullptr) return false;
    return xSemaphoreTake(_mutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

void IoTXClass::unlock() {
    if (_mutex != nullptr) xSemaphoreGive(_mutex);
}

void IoTXClass::reconnectWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[IoTX] WiFi reconnecting...");
        WiFi.disconnect();
        WiFi.begin(_wifiSSID, _wifiPassword);
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(300);
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[IoTX] WiFi reconnected: %s\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.println("[IoTX] WiFi reconnect failed");
        }
    }
}

void IoTXClass::printStatus() {
    Serial.println("=== IoTX Status ===");
    Serial.printf("  WiFi: %s\n", isWiFiConnected() ? "Connected" : "Disconnected");
    if (isWiFiConnected()) {
        Serial.printf("  IP:   %s\n", WiFi.localIP().toString().c_str());
    }
    Serial.printf("  Firebase: %s\n", isFirebaseReady() ? "Ready" : "Not initialized");
    Serial.println("===================");
}

// ============================================================
// IoTXSensor
// ============================================================

IoTXSensor::IoTXSensor(const char* firebasePath) : _path(firebasePath) {}

bool IoTXSensor::write(float value) {
    if (!IoTX.lock()) return false;
    bool ok = Firebase.setFloat(IoTX._fbdo, _path, value);
    IoTX.unlock();
    if (!ok) Serial.printf("[IoTX] Sensor write failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    return ok;
}

float IoTXSensor::read() {
    float val = 0;
    if (!IoTX.lock()) return val;
    if (Firebase.getFloat(IoTX._fbdo, _path)) {
        val = IoTX._fbdo.floatData();
    } else {
        Serial.printf("[IoTX] Sensor read failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    }
    IoTX.unlock();
    return val;
}

const char* IoTXSensor::getPath() const { return _path; }

// ============================================================
// IoTXSwitch
// ============================================================

IoTXSwitch::IoTXSwitch(const char* firebasePath)
    : _path(firebasePath), _pin(-1), _activeLow(false),
      _lastState(false), _pinAttached(false) {}

bool IoTXSwitch::read() {
    if (!IoTX.lock()) return _lastState;
    if (Firebase.getBool(IoTX._fbdo, _path)) {
        _lastState = IoTX._fbdo.boolData();
    } else {
        Serial.printf("[IoTX] Switch read failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    }
    IoTX.unlock();
    return _lastState;
}

bool IoTXSwitch::write(bool state) {
    if (!IoTX.lock()) return false;
    bool ok = Firebase.setBool(IoTX._fbdo, _path, state);
    IoTX.unlock();
    if (ok) _lastState = state;
    else Serial.printf("[IoTX] Switch write failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    return ok;
}

void IoTXSwitch::attachPin(int pin, bool activeLow) {
    _pin = pin;
    _activeLow = activeLow;
    _pinAttached = true;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, _activeLow ? HIGH : LOW);
}

void IoTXSwitch::sync() {
    bool state = read();
    if (_pinAttached) {
        digitalWrite(_pin, (state ^ _activeLow) ? HIGH : LOW);
    }
}

bool IoTXSwitch::getState() const { return _lastState; }
const char* IoTXSwitch::getPath() const { return _path; }

// ============================================================
// IoTXSlider
// ============================================================

IoTXSlider::IoTXSlider(const char* firebasePath)
    : _path(firebasePath), _pin(-1), _minVal(0), _maxVal(100),
      _pinAttached(false) {}

float IoTXSlider::read() {
    float val = 0;
    if (!IoTX.lock()) return val;
    if (Firebase.getFloat(IoTX._fbdo, _path)) {
        val = IoTX._fbdo.floatData();
    } else {
        Serial.printf("[IoTX] Slider read failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    }
    IoTX.unlock();
    return val;
}

bool IoTXSlider::write(float value) {
    if (!IoTX.lock()) return false;
    bool ok = Firebase.setFloat(IoTX._fbdo, _path, value);
    IoTX.unlock();
    if (!ok) Serial.printf("[IoTX] Slider write failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    return ok;
}

void IoTXSlider::attachPin(int pin, float minVal, float maxVal) {
    _pin = pin;
    _minVal = minVal;
    _maxVal = maxVal;
    _pinAttached = true;
    ledcAttach(_pin, 5000, 8);
}

void IoTXSlider::sync() {
    float val = read();
    if (_pinAttached) {
        float clamped = constrain(val, _minVal, _maxVal);
        uint8_t duty = (uint8_t)map((long)(clamped * 100), (long)(_minVal * 100), (long)(_maxVal * 100), 0, 255);
        ledcWrite(_pin, duty);
    }
}

const char* IoTXSlider::getPath() const { return _path; }

// ============================================================
// IoTXDisplay
// ============================================================

IoTXDisplay::IoTXDisplay(const char* firebasePath) : _path(firebasePath) {}

bool IoTXDisplay::write(const char* text) {
    if (!IoTX.lock()) return false;
    bool ok = Firebase.setString(IoTX._fbdo, _path, text);
    IoTX.unlock();
    if (!ok) Serial.printf("[IoTX] Display write failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    return ok;
}

bool IoTXDisplay::write(const String& text) {
    return write(text.c_str());
}

bool IoTXDisplay::printf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    return write(buf);
}

String IoTXDisplay::read() {
    String val;
    if (!IoTX.lock()) return val;
    if (Firebase.getString(IoTX._fbdo, _path)) {
        val = IoTX._fbdo.stringData();
    } else {
        Serial.printf("[IoTX] Display read failed %s: %s\n", _path, IoTX._fbdo.errorReason().c_str());
    }
    IoTX.unlock();
    return val;
}

const char* IoTXDisplay::getPath() const { return _path; }

// ============================================================
// IoTXHardwareMonitor
// ============================================================

IoTXHardwareMonitor::IoTXHardwareMonitor(const char* basePath) : _basePath(basePath) {}

bool IoTXHardwareMonitor::writeCPU(float percent) {
    String path = String(_basePath) + "/cpu";
    if (!IoTX.lock()) return false;
    bool ok = Firebase.setFloat(IoTX._fbdo, path.c_str(), percent);
    IoTX.unlock();
    return ok;
}

bool IoTXHardwareMonitor::writeMemory(float percent) {
    String path = String(_basePath) + "/memory";
    if (!IoTX.lock()) return false;
    bool ok = Firebase.setFloat(IoTX._fbdo, path.c_str(), percent);
    IoTX.unlock();
    return ok;
}

bool IoTXHardwareMonitor::writeDisk(float percent) {
    String path = String(_basePath) + "/disk";
    if (!IoTX.lock()) return false;
    bool ok = Firebase.setFloat(IoTX._fbdo, path.c_str(), percent);
    IoTX.unlock();
    return ok;
}

bool IoTXHardwareMonitor::writeAll(float cpu, float memory, float disk) {
    return writeCPU(cpu) && writeMemory(memory) && writeDisk(disk);
}

const char* IoTXHardwareMonitor::getPath() const { return _basePath; }
