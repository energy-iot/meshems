#pragma once
#include <ArduinoJson.h>

struct SunSpecModel213Base {
    uint16_t model_id = 213;
    uint16_t length = 86;

    float AphA = 0.0, AphB = 0.0, AphC = 0.0;
    float PhVphA = 0.0, PhVphB = 0.0, PhVphC = 0.0;
    float PFphA = 0.0, PFphB = 0.0, PFphC = 0.0;
    float VarphA = 0.0, VarphB = 0.0, VarphC = 0.0;
    float VAphA = 0.0, VAphB = 0.0, VAphC = 0.0;
    float WphA = 0.0, WphB = 0.0, WphC = 0.0;
    float Wh = 0.0, WhphA = 0.0, WhphB = 0.0, WhphC = 0.0;
    float Hz = 0.0;

    // THD
    float THD_VphA = 0.0, THD_VphB = 0.0, THD_VphC = 0.0;
    float THD_IphA = 0.0, THD_IphB = 0.0, THD_IphC = 0.0;

    // Totals
    float Tot15mWhImport = 0.0, Tot15mWhExport = 0.0;
    float TotHrWhImport = 0.0, TotHrWhExport = 0.0;
    float TotDayWhImport = 0.0, TotDayWhExport = 0.0;
    float TotTOUCWhImport = 0.0, TotTOUCWhExport = 0.0;
    float TotEventWhImport = 0.0, TotEventWhExport = 0.0;
    float TotWhImport = 0.0, TotWhExport = 0.0;

    void toJson(JsonDocument& doc) const {
        doc["model_id"] = model_id;
        doc["length"] = length;

        doc["Hz"] = Hz;
        doc["AphA"] = AphA; doc["AphB"] = AphB; doc["AphC"] = AphC;
        doc["PhVphA"] = PhVphA; doc["PhVphB"] = PhVphB; doc["PhVphC"] = PhVphC;
        doc["PFphA"] = PFphA; doc["PFphB"] = PFphB; doc["PFphC"] = PFphC;
        doc["VarphA"] = VarphA; doc["VarphB"] = VarphB; doc["VarphC"] = VarphC;
        doc["VAphA"] = VAphA; doc["VAphB"] = VAphB; doc["VAphC"] = VAphC;
        doc["WphA"] = WphA; doc["WphB"] = WphB; doc["WphC"] = WphC;

        doc["THD_VphA"] = THD_VphA; doc["THD_VphB"] = THD_VphB; doc["THD_VphC"] = THD_VphC;
        doc["THD_IphA"] = THD_IphA; doc["THD_IphB"] = THD_IphB; doc["THD_IphC"] = THD_IphC;

        doc["TotHrWhImport"] = TotHrWhImport;
        doc["TotHrWhExport"] = TotHrWhExport;
        doc["TotTOUCWhImport"] = TotTOUCWhImport;
        doc["TotTOUCWhExport"] = TotTOUCWhExport;
        doc["TotDayImport"] = TotDayWhImport;
        doc["TotDayExport"] = TotDayWhExport;
        doc["TotWhImport"] = TotWhImport;
        doc["TotWhExport"] = TotWhExport;
    }
};