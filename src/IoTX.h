#ifndef IOTX_H
#define IOTX_H

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// ============================================================
// IoTXConfig
// ============================================================
struct IoTXConfig {
    const char* wifiSSID;
    const char* wifiPassword;
    const char* firebaseHost;
    const char* firebaseAuth;
};

// ============================================================
// IoTXSensor - Push sensor readings (number) to dashboard
// ============================================================
class IoTXSensor {
public:
    IoTXSensor(const char* firebasePath);
    bool write(float value);
    float read();
    const char* getPath() const;
private:
    const char* _path;
};

// ============================================================
// IoTXSwitch - Boolean control (switch/led widgets)
// ============================================================
class IoTXSwitch {
public:
    IoTXSwitch(const char* firebasePath);
    bool read();
    bool write(bool state);
    void attachPin(int pin, bool activeLow = false);
    void sync();
    bool getState() const;
    const char* getPath() const;
private:
    const char* _path;
    int _pin;
    bool _activeLow;
    bool _lastState;
    bool _pinAttached;
};

// ============================================================
// IoTXSlider - Numeric range control (range-slider widget)
// ============================================================
class IoTXSlider {
public:
    IoTXSlider(const char* firebasePath);
    float read();
    bool write(float value);
    void attachPin(int pin, float minVal = 0, float maxVal = 100);
    void sync();
    const char* getPath() const;
private:
    const char* _path;
    int _pin;
    float _minVal;
    float _maxVal;
    bool _pinAttached;
};

// ============================================================
// IoTXButton - Push button control (button widget)
// ============================================================
class IoTXButton {
public:
    IoTXButton(const char* firebasePath);
    bool read();
    bool write(bool state);
    bool toggle();
    bool getState() const;
    const char* getPath() const;
private:
    const char* _path;
    bool _lastState;
};

// ============================================================
// IoTXDisplay - Text display (lcd-text widget)
// ============================================================
class IoTXDisplay {
public:
    IoTXDisplay(const char* firebasePath);
    bool write(const char* text);
    bool write(const String& text);
    bool printf(const char* format, ...);
    String read();
    const char* getPath() const;
private:
    const char* _path;
};

// ============================================================
// IoTXHardwareMonitor - CPU/Memory/Disk (hw-manager widget)
// ============================================================
class IoTXHardwareMonitor {
public:
    IoTXHardwareMonitor(const char* basePath = "/hardware");
    bool writeCPU(float percent);
    bool writeMemory(float percent);
    bool writeDisk(float percent);
    bool writeAll(float cpu, float memory, float disk);
    const char* getPath() const;
private:
    const char* _basePath;
};

// ============================================================
// IoTXClass - Main singleton
// ============================================================
class IoTXClass {
public:
    static constexpr uint8_t FBDO_POOL_SIZE = 3;

    IoTXClass();
    bool begin(const IoTXConfig& config);
    bool isConnected() const;
    bool isWiFiConnected() const;
    bool isFirebaseReady() const;
    FirebaseData& getFirebaseData();
    void reconnectWiFi();
    void printStatus();

    int acquireFirebaseData(uint32_t timeoutMs = 1000);
    void releaseFirebaseData(int index);

private:
    FirebaseData _fbdoPool[FBDO_POOL_SIZE];
    SemaphoreHandle_t _poolMutexes[FBDO_POOL_SIZE];
    FirebaseAuth _auth;
    FirebaseConfig _config;
    bool _initialized;
    const char* _wifiSSID;
    const char* _wifiPassword;

    friend class IoTXSensor;
    friend class IoTXSwitch;
    friend class IoTXSlider;
    friend class IoTXButton;
    friend class IoTXDisplay;
    friend class IoTXHardwareMonitor;
};

extern IoTXClass IoTX;

#endif
