#pragma once
#include <ArduinoJson.h>

/*
Key Features of SunSpec Model 213
Model ID: 213
Configuration: Three-phase WYE (ABCN)
Register Length: 124 registers

Sunspec Data Points Include:
Total and per-phase currents (A, AphA, AphB, AphC)
Line-to-neutral voltages (PhVphA, PhVphB, PhVphC)
Line-to-line voltages (PhVab, PhVbc, PhVca)
Active power (W, WphA, WphB, WphC)
Reactive power (Var, VarphA, VarphB, VarphC)
Apparent power (VA, VaphA, VaphB, VaphC)
Power factor (PF, PFphA, PFphB, PFphC)
Frequency (Hz)
Energy accumulations (Wh, WhphA, WhphB, WhphC, etc.
*/

struct SunSpecModel213 {
    uint16_t model_id = 213;
    uint16_t length = 85;

    float AphA = 0.0;
    float AphB = 0.0;
    float AphC = 0.0;

    float PhVphA = 0.0;
    float PhVphB = 0.0;
    float PhVphC = 0.0;

    float PFphA = 0.0;
    float PFphB = 0.0;
    float PFphC = 0.0;

    float VarphA = 0.0;
    float VarphB = 0.0;
    float VarphC = 0.0;

    float VAphA = 0.0;
    float VAphB = 0.0;
    float VAphC = 0.0;

    float WphA = 0.0;
    float WphB = 0.0;
    float WphC = 0.0;

    float Wh = 0.0;
    float WhphA = 0.0;
    float WhphB = 0.0;
    float WhphC = 0.0;

    //TODO add harmonics - see wmac.cloud DTM

    float Hz = 0.0;

    float Tot15mWhAImport = 0.0;
    float Tot15mWhAExport = 0.0;
    float TotHrWhAImport = 0.0;
    float TotHrWhAExport = 0.0;
    float TotDayWhAImport = 0.0;
    float TotDayWhAExport = 0.0;
    float TotTOUCAWhImport = 0.0;
    float TotTOUCAWhExport = 0.0;
    float TotEventAWhImport = 0.0;
    float TotEventAWhExport = 0.0;
    float TotWhAImport = 0.0;
    float TotWhAExport = 0.0;
        
    float Tot15mWhBImport = 0.0;
    float Tot15mWhBExport = 0.0;
    float TotHrWhBImport = 0.0;
    float TotHrWhBExport = 0.0;
    float TotDayWhBImport = 0.0;
    float TotDayWhBExport = 0.0;
    float TotTOUCBWhImport = 0.0;
    float TotTOUCBWhExport = 0.0;
    float TotEventBWhImport = 0.0;
    float TotEventBWhExport = 0.0;
    float TotWhBImport = 0.0;
    float TotWhBExport = 0.0;
    
    float Tot15mWhCImport = 0.0;
    float Tot15mWhCExport = 0.0;
    float TotHrWhCImport = 0.0;
    float TotHrWhCExport = 0.0;
    float TotDayWhCImport = 0.0;
    float TotDayWhCExport = 0.0;
    float TotTOUCWhCImport = 0.0;
    float TotTOUCWhCExport = 0.0;
    float TotEventWhCImport = 0.0;
    float TotEventWhCExport = 0.0;
    float TotWhCImport = 0.0;
    float TotWhCExport = 0.0;

    float Tot15mWhImport = 0.0;
    float Tot15mWhExport = 0.0;
    float TotHrWhImport = 0.0;
    float TotHrWhExport = 0.0;
    float TotDayWhImport = 0.0;
    float TotDayWhExport = 0.0;
    float TotTOUCWhImport = 0.0;
    float TotTOUCWhExport = 0.0;
    float TotEventWhImport = 0.0;
    float TotEventWhExport = 0.0;
    float TotWhImport = 0.0;
    float TotWhExport = 0.0;

    //TODO 3Ph Harmonics reports

    //include 3phases unbalanced useage per hr pr day per wk

    //TODO Leakage report RCD min, max, avg, mean variance each for DC milliamp, AC milliamp, leakage wattage

    //TODO stats of reports, faults, devops

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
        
    // TODO need to have per phase reports and total all 3 phases
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
