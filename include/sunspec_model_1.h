#ifndef SUNSPEC_MODEL_1_H
#define SUNSPEC_MODEL_1_H

#include <ArduinoJson.h>
/*
This Model 1 is good for its own subtopic in OPENAMI to identify the nameplate rating of the streetpoleEMS maker and skU
model and versioning of Hardware integrated (for example 3 phase breakout vs single phase breakout and version of software running at ems
how many tenants serve
alsoo OAM alarms sumary such as caution of any export of energy active from tenants,
a local  faults summary like leakage performance issue/alarm or harmonics performance issue/alarm
*/
struct SunSpecModel1_EMS { //  EMS subpanel as a cookie cutter prebuilt, wired, software QA tested 
    uint16_t model_id = 1;   // SunSpec Common Model ID
    uint16_t length = 125;    // Default length for Model 1 (can adjust per device)

    // Device Identification Fields
    char Mn[33] = "NESL EIOT Prototype";    // Manufacturer
    char Md[33] = "IP65_5KVA";    // Model
    char Opt[64] = "3Tenant_Meter_Hiking_DDS328_ZN/S, LeakageRCMperPhase";   // Options
    char Vr[17] = "01.1";    // EMS subpanel Version
    char SN[17] = "";    // TODO Serial Number - put EMS full macid here 
    char DA[17] = "";    // TODO Device Address (or other ID) - put shortenend unique MACID
    char Alarms[64] = "";    // list of  subpanel alarms relevent to a truck roll event 

    void clear() {
        strcpy(Mn, "");
        strcpy(Md, "");
        strcpy(Opt, "");
        strcpy(Vr, "");
        strcpy(SN, "");
        strcpy(DA, "");
        strcpy(Alarms, "");
    }

    void toJson(JsonDocument& doc) const {
        doc["model_id"] = model_id;
        doc["length"] = length;
        doc["Mn"] = Mn;
        doc["Md"] = Md;
        doc["Opt"] = Opt;
        doc["Vr"] = Vr;
        doc["SN"] = SN;
        doc["DA"] = DA;
        doc["ALARMS"] = Alarms;
    }

    void print(Stream& stream) const {
        stream.println(F("SunSpec Model 1 - Common Block"));
        stream.print(F("Manufacturer: ")); stream.println(Mn);
        stream.print(F("Model: ")); stream.println(Md);
        stream.print(F("Options: ")); stream.println(Opt);
        stream.print(F("Version: ")); stream.println(Vr);
        stream.print(F("Serial Number: ")); stream.println(SN);
        stream.print(F("Device Address: ")); stream.println(DA);
        stream.print(F("ALARMS: ")); stream.println(Alarms);
    }
};

#endif // SUNSPEC_MODEL_1_H
