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
Energy accumulations (Wh, WhphA, WhphB, WhphC, etc.)
THD and 1st to 15th voltage harmonics integrated

Notes:
    THD_VphX and THD_IphX represent Total Harmonic Distortion for voltage and current per phase.

    Harmonics arrays (Harmonics_VphX, Harmonics_IphX) are indexed from 0–14 to represent the 1st–15th harmonics.

    The .toJson() function now includes arrays serialized to JSON using createNestedArray.

OPTIONAL TODO:

    Per-order harmonics labeled explicitly (e.g., "H1":, "H2":, …).

    RMS or % harmonics instead of absolute.

    This broken out per MQTT topic structure.
*/

struct SunSpecModel213 {
    uint16_t model_id = 213;
    uint16_t length = 85;  //TODO confirm length

    float AphA = 0.0, AphB = 0.0, AphC = 0.0;
    float PhVphA = 0.0, PhVphB = 0.0, PhVphC = 0.0;
    float PFphA = 0.0, PFphB = 0.0, PFphC = 0.0;
    float VarphA = 0.0, VarphB = 0.0, VarphC = 0.0;
    float VAphA = 0.0, VAphB = 0.0, VAphC = 0.0;
    float WphA = 0.0, WphB = 0.0, WphC = 0.0;

    float Wh = 0.0, WhphA = 0.0, WhphB = 0.0, WhphC = 0.0;

    float Hz = 0.0;

    float THD_VphA = 0.0, THD_VphB = 0.0, THD_VphC = 0.0;
    float THD_IphA = 0.0, THD_IphB = 0.0, THD_IphC = 0.0;
// TODO move out below detailed Harmonic distortion per phase (1st–15th order + THD) report as its own per phase report at adaptive rate of publish 
    float Harmonics_VphA[15] = {0};  // 1st to 15th voltage harmonics, phase A
    float Harmonics_VphB[15] = {0};  // phase B
    float Harmonics_VphC[15] = {0};  // phase C

    float Harmonics_IphA[15] = {0};  // 1st to 15th current harmonics, phase A
    float Harmonics_IphB[15] = {0};  // phase B
    float Harmonics_IphC[15] = {0};  // phase C

    // 3 phase Energy totalizers
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

    //TODO include new novel fields of transient grid energy excess supply, store, transform, and demand load import and export "potentials"
    //This offers the GroupLead EMS policy decision point to make much more meaningful load  shave, shift, balance, policy serving schedules
    //  that include a well balanced grid excess supply and excess load demand response transient reponse by distributing these
    // transient condition handling schedules to StreetPoleEMS edges
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