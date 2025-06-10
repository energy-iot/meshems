#pragma once
#include <ArduinoJson.h>

/*
SunSpec Model 11 - for use by Single Phase AC Meter (per tenant for example) with Harmonics and THD Support added
Model ID: 11
*/

struct SunSpecModel11 { // used for single phase per tenant meter reports - billable 15 min interval energy use case
    uint16_t model_id = 11;
    uint16_t length = 38; // Actual modbus length, not including harmonics

    // Real-time measurements
    float A = 0.0;         // RMS Current
    float PhV = 0.0;       // RMS Voltage
    float W = 0.0;         // Active Power
    float VA = 0.0;        // Apparent Power
    float Var = 0.0;       // Reactive Power
    float PF = 0.0;        // Power Factor
    float Hz = 0.0;        // Frequency

    // Accumulated Energy
    float TotWhImport = 0.0;
    float TotWhExport = 0.0;
    float TotVarhImport = 0.0;
    float TotVarhExport = 0.0;
    float TotAh = 0.0;
    // accumulations in last 15 min, 1 hr, 1 day, 1month
    float Tot15mWhImport = 0.0;
    float Tot15mWhExport = 0.0;
    float TotHrWhImport = 0.0;
    float TotHrWhExport = 0.0;
    float TotDayWhImport = 0.0;
    float TotDayWhExport = 0.0;
    float TotMnthWhImport = 0.0;
    float TotMnthWhExport = 0.0;

    // TBD not to many single phase meters support it, optional Harmonics (1st through 15th)
    float currentHarmonics[15] = {0.0}; // A_H1 through A_H15
    float voltageHarmonics[15] = {0.0}; // V_H1 through V_H15

    // Optional Total Harmonic Distortion
    float THD_A = 0.0; // THD Current (%)
    float THD_V = 0.0; // THD Voltage (%)

    void toJson(JsonDocument& doc) const {
        doc["model_id"] = model_id;
        doc["length"] = length;

        doc["Hz"] = Hz;
        doc["A"] = A;
        doc["PhV"] = PhV;
        doc["W"] = W;
        doc["VA"] = VA;
        doc["Var"] = Var;
        doc["PF"] = PF;

        doc["TotWhImport"] = TotWhImport;
        doc["TotWhExport"] = TotWhExport;
        doc["TotVarhImport"] = TotVarhImport;
        doc["TotVarhExport"] = TotVarhExport;
        doc["TotAh"] = TotAh;

        doc["Tot15mWhImport"] = Tot15mWhImport;
        doc["Tot15mWhExport"] = Tot15mWhExport;
        doc["TotHrWhImport"] = TotHrWhImport;
        doc["TotHrWhExport"] = TotHrWhExport;
        doc["TotDayWhImport"] = TotDayWhImport;
        doc["TotDayWhExport"] = TotDayWhExport;

        // Labelled Harmonics
        JsonObject harmonics = doc.createNestedObject("harmonics");

        for (int i = 0; i < 15; ++i) {
            String keyA = "A_H" + String(i + 1);
            String keyV = "V_H" + String(i + 1);
            harmonics[keyA] = currentHarmonics[i];
            harmonics[keyV] = voltageHarmonics[i];
        }

        // Add THD metrics
        doc["THD_A"] = THD_A;
        doc["THD_V"] = THD_V;
    }
};