#pragma once
#include <ArduinoJson.h>

/*
Key Features of Leakage
TYPE B
DC milliamp leakage to measure, monitor for change and report growing leakage and isolating if possible the powerflor\w characteristics existing during changed leakage conditions
Worst case the fault line on the leakage sensors are set for lfesafety  levels when autonomous sub 300 msec response is to remove power flow and go open circuit protection
t\These leakage sensors report when leakage is removed and each StreetPoleEMS independantly can remotely or autonomously restore local power flow transfers

*/

    
struct LeakageChannel {
    float value_mA = 0.0f;
    float threshold_mA = 30.0f;
    float lastFaultValue_mA = 0.0f;
    uint32_t lastFaultTimeMs = 0;
    bool inFault = false;

    void update(float newValue_mA) {
        value_mA = newValue_mA;
        if (value_mA >= threshold_mA) {
            if (!inFault) {
                lastFaultTimeMs = millis();
                lastFaultValue_mA = value_mA;
            }
            inFault = true;
        } else {
            inFault = false;
        }
    }

    void resetFault() {
        inFault = false;
        lastFaultValue_mA = 0.0f;
        lastFaultTimeMs = 0;
    }

    void print(const char* label, Stream& out = Serial) const {
        out.printf("[%s] Value: %.2f mA | Threshold: %.2f mA | In Fault: %s | Last Fault: %.2f mA at %lu ms\n",
                   label, value_mA, threshold_mA,
                   inFault ? "YES" : "NO",
                   lastFaultValue_mA, lastFaultTimeMs);
    }

    void toJson(JsonObject obj) const {
        obj["value_mA"] = value_mA;
        obj["threshold_mA"] = threshold_mA;
        obj["lastFaultValue_mA"] = lastFaultValue_mA;
        obj["lastFaultTimeMs"] = lastFaultTimeMs;
        obj["inFault"] = inFault;
    }
};

struct LeakageModel { // TODO  review leakage specs and consult with SME's
    uint16_t model_id = 999;
    uint16_t length = 10;
    LeakageChannel acSinusoidal;   // Type A AC sinusoidal
    LeakageChannel acPulsating;    // Type A AC pulsating (rectified)
    LeakageChannel dc;             // Type B DC residual

    uint32_t lastUpdateMs = 0;
    bool valid = false;

    void clear() {
        acSinusoidal = LeakageChannel();
        acPulsating = LeakageChannel();
        dc = LeakageChannel();
        lastUpdateMs = millis();
        valid = false;
    }

    void updateAll(float acSin_mA, float acPulse_mA, float dc_mA) {
        acSinusoidal.update(acSin_mA);
        acPulsating.update(acPulse_mA);
        dc.update(dc_mA);
        lastUpdateMs = millis();
        valid = true;
    }

    void toJson(JsonDocument& doc) const {
    doc["model_id"] = model_id;
    doc["length"] = length;
    doc["lastUpdateMs"] = lastUpdateMs;

    // Modern way to create nested objects
    JsonObject acSin = doc["acSinusoidal"].to<JsonObject>();
    acSinusoidal.toJson(acSin);
    JsonObject acPulse = doc["acPulsating"].to<JsonObject>();
    acPulsating.toJson(acPulse);
    JsonObject dcObj = doc["dc"].to<JsonObject>();
    dc.toJson(dcObj);
    }
};
