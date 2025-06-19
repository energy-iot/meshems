#pragma once
#include <ArduinoJson.h>

struct SunSpecModel213Harmonics {
    uint16_t model_id = 213;
    uint16_t length = 180;

    float Harmonics_VphA[15] = {0};
    float Harmonics_VphB[15] = {0};
    float Harmonics_VphC[15] = {0};

    float Harmonics_IphA[15] = {0};
    float Harmonics_IphB[15] = {0};
    float Harmonics_IphC[15] = {0};

    void toJson(JsonDocument& doc) const {
        doc["model_id"] = model_id;
        doc["length"] = length;
        
        JsonArray hVA = doc["Harmonics_VphA"].to<JsonArray>();
        JsonArray hVB = doc["Harmonics_VphB"].to<JsonArray>();
        JsonArray hVC = doc["Harmonics_VphC"].to<JsonArray>();
        JsonArray hIA = doc["Harmonics_IphA"].to<JsonArray>();
        JsonArray hIB = doc["Harmonics_IphB"].to<JsonArray>();
        JsonArray hIC = doc["Harmonics_IphC"].to<JsonArray>();

        for (int i = 0; i < 15; ++i) {
            hVA.add(Harmonics_VphA[i]);
            hVB.add(Harmonics_VphB[i]);
            hVC.add(Harmonics_VphC[i]);
            hIA.add(Harmonics_IphA[i]);
            hIB.add(Harmonics_IphB[i]);
            hIC.add(Harmonics_IphC[i]);
        }
    }

    void clear() {
        memset(Harmonics_VphA, 0, sizeof(Harmonics_VphA));
        memset(Harmonics_VphB, 0, sizeof(Harmonics_VphB));
        memset(Harmonics_VphC, 0, sizeof(Harmonics_VphC));
        memset(Harmonics_IphA, 0, sizeof(Harmonics_IphA));
        memset(Harmonics_IphB, 0, sizeof(Harmonics_IphB));
        memset(Harmonics_IphC, 0, sizeof(Harmonics_IphC));
    }

    void print() const {
        Serial.println(F("Harmonics_VphA:")); for (auto v : Harmonics_VphA) Serial.println(v);
        Serial.println(F("Harmonics_VphB:")); for (auto v : Harmonics_VphB) Serial.println(v);
        Serial.println(F("Harmonics_VphC:")); for (auto v : Harmonics_VphC) Serial.println(v);
        Serial.println(F("Harmonics_IphA:")); for (auto v : Harmonics_IphA) Serial.println(v);
        Serial.println(F("Harmonics_IphB:")); for (auto v : Harmonics_IphB) Serial.println(v);
        Serial.println(F("Harmonics_IphC:")); for (auto v : Harmonics_IphC) Serial.println(v);
    }
};