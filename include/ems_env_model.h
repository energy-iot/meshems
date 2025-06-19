/*
 * EMS_ENV_Model
 * ------------------------------
 * Captures environmental runtime conditions for EMS subpanels.
 * Includes support for:
 *  - Temperature and humidity via Modbus node 99 or I2C sensor
 *  - Tamper switch status via GPIO input
 *  - Time tracking for door open duration and last updates
 *  - JSON export for MQTT publication intervals
 *
 * Created by: NESL.Energy Engineering Team
 * Created on: 2024-06-15
 * Modified by: Glenn G Algie, 2025-06-15
 */

#ifndef EMS_ENV_MODEL_H
#define EMS_ENV_MODEL_H

#include <ArduinoJson.h>
#include <Wire.h>

struct EMS_ENV_Model {
    uint16_t model_id = 998;   // Custom EMS environment model ID
    uint16_t length = 17;       // Adjustable length as needed

    // Cached environmental state
    float temperature_C = 0.0;
    float humidity_percent = 0.0;
    bool door_open = false;     // True = open, False = closed
    unsigned long timestamp_ms = 0; // Time of last update in millis()

    // Internal cache timestamps
    unsigned long last_modbus_update_ms = 0;
    unsigned long last_gpio_update_ms = 0;

    // Door open timing
    unsigned long door_open_start_ms = 0;
    unsigned long door_open_duration_ms = 0;

    // GPIO and Modbus settings
    int door_switch_gpio = -1;  // to be set externally

    // Clear cached values
    void clear() {
        temperature_C = 0.0;
        humidity_percent = 0.0;
        door_open = false;
        timestamp_ms = 0;
        last_modbus_update_ms = 0;
        last_gpio_update_ms = 0;
        door_open_start_ms = 0;
        door_open_duration_ms = 0;
    }

    // JSON export for MQTT
    void toJson(JsonDocument& doc) const {
        doc["model_id"] = model_id;
        doc["length"] = length;
        doc["temperature_C"] = temperature_C;
        doc["humidity_percent"] = humidity_percent;
        doc["door_open"] = door_open;
        doc["timestamp_ms"] = timestamp_ms;
        doc["last_modbus_update_ms"] = last_modbus_update_ms;
        doc["last_gpio_update_ms"] = last_gpio_update_ms;
        doc["door_open_duration_ms"] = door_open_duration_ms;
    }

    // Print debug info
    void print(Stream& stream = Serial) const {
        stream.println(F("EMS Environmental Conditions"));
        stream.print(F("Temperature (C): ")); stream.println(temperature_C);
        stream.print(F("Humidity (%): ")); stream.println(humidity_percent);
        stream.print(F("Door Open: ")); stream.println(door_open ? "YES" : "NO");
        stream.print(F("Timestamp (ms): ")); stream.println(timestamp_ms);
        stream.print(F("Last Modbus Update (ms): ")); stream.println(last_modbus_update_ms);
        stream.print(F("Last GPIO Update (ms): ")); stream.println(last_gpio_update_ms);
        stream.print(F("Door Open Duration (ms): ")); stream.println(door_open_duration_ms);
    }

    // Capture via GPIO tamper switch
    void readDoorState() {
        if (door_switch_gpio >= 0) {
            pinMode(door_switch_gpio, INPUT_PULLDOWN);
            bool current_state = digitalRead(door_switch_gpio);

            if (current_state && !door_open) {
                // Transition from closed to open
                door_open_start_ms = millis();
            } else if (!current_state && door_open) {
                // Transition from open to closed
                door_open_duration_ms = millis() - door_open_start_ms;
                door_open_start_ms = 0;
            }

            door_open = current_state;
            last_gpio_update_ms = millis();
            timestamp_ms = millis();
        }
    }

    // Capture via Modbus node 99 (mocked for now)
    void readModbusSensor() {
        // TODO: replace with actual Modbus client logic
        temperature_C = 25.2;
        humidity_percent = 48.7;
        last_modbus_update_ms = millis();
        timestamp_ms = millis();
    }
};

#endif // EMS_ENV_MODEL_H