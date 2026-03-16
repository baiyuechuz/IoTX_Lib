# Firebase Path ↔ Dashboard Widget Mapping

## Default Paths

These are the default `firebasePath` values used by IoTX Website dashboard widgets. Use the same path in your code to connect automatically.

| Firebase Path | Data Type | Library Class | Dashboard Widget |
|---|---|---|---|
| `/sensors/temperature` | `float` | `IoTXSensor` | Temperature |
| `/sensors/humidity` | `float` | `IoTXSensor` | Humidity |
| `/sensors/smoke` | `float` | `IoTXSensor` | Smoke |
| `/sensors/progress` | `float` | `IoTXSensor` | Progress Bar |
| `/sensors/data` | `float` | `IoTXSensor` | Chart |
| `/controls/switch` | `bool` | `IoTXSwitch` | Switch |
| `/controls/led` | `bool` | `IoTXSwitch` | LED |
| `/controls/range-slider` | `float` | `IoTXSlider` | Range Slider |
| `/display/lcd-text` | `string` | `IoTXDisplay` | LCD Text |
| `/hardware/cpu` | `float` | `IoTXHardwareMonitor` | HW Manager (CPU) |
| `/hardware/memory` | `float` | `IoTXHardwareMonitor` | HW Manager (RAM) |
| `/hardware/disk` | `float` | `IoTXHardwareMonitor` | HW Manager (Disk) |

## Custom Paths

Paths are **fully configurable**. You can use any path — just make sure the dashboard widget's `firebasePath` setting matches what you pass in your code.

```cpp
// These work as long as the dashboard widget uses the same path
IoTXSensor roomTemp("/home/living-room/temperature");
IoTXSwitch garageLight("/home/garage/light");
IoTXDisplay kitchenLCD("/home/kitchen/display");
```

## Firebase RTDB Structure

When you use the default paths, your Firebase Realtime Database will look like:

```json
{
  "sensors": {
    "temperature": 25.5,
    "humidity": 60.2,
    "smoke": 12.3,
    "progress": 75.0,
    "data": 42.0
  },
  "controls": {
    "switch": true,
    "led": false,
    "range-slider": 50.0
  },
  "display": {
    "lcd-text": "Hello World!"
  },
  "hardware": {
    "cpu": 45.2,
    "memory": 62.0,
    "disk": 80.5
  }
}
```

## Data Types

| Library Class | Firebase Type | ESP32 Type | Dashboard Reads As |
|---|---|---|---|
| `IoTXSensor` | number | `float` | `number` |
| `IoTXSwitch` | boolean | `bool` | `boolean` (`true`/`false`) |
| `IoTXSlider` | number | `float` | `number` |
| `IoTXDisplay` | string | `String` / `const char*` | `string` |
| `IoTXHardwareMonitor` | number | `float` | `number` (0–100) |

> **Note:** `IoTXSwitch` uses native booleans (`true`/`false`), not strings like `"on"`/`"off"`. The IoTX Dashboard expects boolean values for Switch and LED widgets.
