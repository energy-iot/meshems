#pragma once
#include <ArduinoJson.h>

struct SunSpecModel213 {
    uint16_t model_id = 213;
    uint16_t length = 85;

    float A_phA = 0.0;
    float A_phB = 0.0;
    float A_phC = 0.0;

    float PhV_phA = 0.0;
    float PhV_phB = 0.0;
    float PhV_phC = 0.0;

    float PF_phA = 0.0;
    float PF_phB = 0.0;
    float PF_phC = 0.0;

    float VAR_phA = 0.0;
    float VAR_phB = 0.0;
    float VAR_phC = 0.0;

    float W_phA = 0.0;
    float W_phB = 0.0;
    float W_phC = 0.0;

    float Hz = 0.0;

    float TotWhImport = 0.0;
    float TotWhExport = 0.0;

    void toJson(JsonDocument& doc) const {
        doc["model_id"] = model_id;
        doc["length"] = length;

        doc["A_phA"] = A_phA; doc["A_phB"] = A_phB; doc["A_phC"] = A_phC;
        doc["PhV_phA"] = PhV_phA; doc["PhV_phB"] = PhV_phB; doc["PhV_phC"] = PhV_phC;
        doc["PF_phA"] = PF_phA; doc["PF_phB"] = PF_phB; doc["PF_phC"] = PF_phC;
        doc["VAR_phA"] = VAR_phA; doc["VAR_phB"] = VAR_phB; doc["VAR_phC"] = VAR_phC;
        doc["W_phA"] = W_phA; doc["W_phB"] = W_phB; doc["W_phC"] = W_phC;
        doc["Hz"] = Hz;
        doc["TotWhImport"] = TotWhImport;
        doc["TotWhExport"] = TotWhExport;
    }
};
