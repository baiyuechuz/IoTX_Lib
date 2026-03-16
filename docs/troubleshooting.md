# Troubleshooting

## Connection Issues

### `[IoTX] WiFi connection timeout`

- Check SSID and password (case-sensitive)
- Ensure the network is 2.4 GHz (ESP32 doesn't support 5 GHz)
- Move closer to the router
- Check that the router isn't blocking new devices

### `[IoTX] Mutex creation failed`

- Not enough heap memory to create a FreeRTOS mutex
- Reduce other memory allocations or use a board with more RAM

### Firebase `permission denied`

- Check Firebase Realtime Database rules
- For testing, use open rules:

```json
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
```

> ⚠️ Use restrictive rules in production.

### Firebase `not found` or `host not found`

- `firebaseHost` should be just the hostname, **without** `https://`
- ✅ `"my-project-default-rtdb.asia-southeast1.firebasedatabase.app"`
- ❌ `"https://my-project-default-rtdb.asia-southeast1.firebasedatabase.app"`

---

## Dashboard Issues

### Values not showing on dashboard

- Ensure the widget's **Firebase Path** setting matches the path in your code exactly
- Check Firebase Console → Realtime Database to verify values are being written
- Try `IoTX.printStatus()` to verify connection

### Switch widget shows `null`

- Old code may have used `"on"`/`"off"` strings
- IoTX uses native booleans (`true`/`false`) — the dashboard expects this format
- Delete the old value in Firebase Console and let IoTX write a fresh boolean

### Values update slowly on dashboard

- Check your `vTaskDelay` intervals — shorter = faster updates
- Firebase free tier has rate limits — don't write faster than every 1 second

---

## Hardware Issues

### Slider PWM not working

- Not all ESP32 pins support LEDC PWM
- Use: GPIO **2, 4, 12–15, 25–27, 32–33**
- Avoid: GPIO 0, 1, 3, 6–11 (used for flash/UART)

### LED not turning on/off

- Check wiring and pin number
- For active-low modules (relays), use `attachPin(pin, true)`
- Verify with `Serial.println(led.getState())` to check the Firebase value

### Crash / reboot loop

- **Stack overflow:** Increase task stack size from `4096` to `8192`
- **Watchdog timeout:** Add `vTaskDelay()` in long loops
- **Before begin():** Don't use IoTX objects before calling `IoTX.begin()`

---

## Serial Monitor Messages

| Message | Meaning |
|---------|---------|
| `[IoTX] WiFi connecting...` | Connecting to WiFi (dots show progress) |
| `[IoTX] WiFi OK: 192.168.x.x` | WiFi connected successfully |
| `[IoTX] WiFi connection timeout` | Failed to connect within 15 seconds |
| `[IoTX] Ready` | WiFi + Firebase + Mutex all initialized |
| `[IoTX] Mutex creation failed` | Not enough memory for FreeRTOS mutex |
| `[IoTX] WiFi reconnecting...` | WiFi dropped, attempting reconnect |
| `[IoTX] WiFi reconnected: 192.168.x.x` | Reconnection successful |
| `[IoTX] WiFi reconnect failed` | Reconnection timed out (10 seconds) |
| `[IoTX] Sensor write failed <path>: <reason>` | Firebase write error |
| `[IoTX] Sensor read failed <path>: <reason>` | Firebase read error |
| `[IoTX] Switch read failed <path>: <reason>` | Firebase read error |
| `[IoTX] Switch write failed <path>: <reason>` | Firebase write error |
| `[IoTX] Slider read failed <path>: <reason>` | Firebase read error |
| `[IoTX] Slider write failed <path>: <reason>` | Firebase write error |
| `[IoTX] Display write failed <path>: <reason>` | Firebase write error |
| `[IoTX] Display read failed <path>: <reason>` | Firebase read error |

---

## Getting Help

1. Check the [Firebase Path Mapping](firebase-paths.md) to verify your paths
2. Use `IoTX.printStatus()` to diagnose connection state
3. Monitor Serial output at 115200 baud for error messages
4. Check Firebase Console to see if values are being written
5. Open an issue on [GitHub](https://github.com/baiyuechuz/IoTX_Lib/issues)
