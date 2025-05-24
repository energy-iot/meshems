#ifndef SUNSPEC_MODEL_1_H
#define SUNSPEC_MODEL_1_H

#include <ArduinoJson.h>

struct SunSpecModel1_SPEMS { // Street Pole EMS as a cookie cutter prebuilt, wired, software QA tested 
    uint16_t model_id = 1;   // SunSpec Common Model ID
    uint16_t length = 66;    // Default length for Model 1 (can adjust per device)

    // Device Identification Fields
    char Mn[33] = "NESL EIOT Prototype";    // Manufacturer
    char Md[33] = "IP65_5KVA_LeakageperPhase";    // Model
    char Opt[64] = "3Tenant_Meter_Hiking_DDS328_ZN/S";   // Options
    char Vr[17] = "01.1";    // Version
    char SN[17] = "";    // TODO Serial Number - put EMS full macid
    char DA[17] = "";    // TODO Device Address (or other ID) - put shortenend unique MACID

    void clear() {
        strcpy(Mn, "");
        strcpy(Md, "");
        strcpy(Opt, "");
        strcpy(Vr, "");
        strcpy(SN, "");
        strcpy(DA, "");
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
    }

    void print(Stream& stream) const {
        stream.println(F("SunSpec Model 1 - Common Block"));
        stream.print(F("Manufacturer: ")); stream.println(Mn);
        stream.print(F("Model: ")); stream.println(Md);
        stream.print(F("Options: ")); stream.println(Opt);
        stream.print(F("Version: ")); stream.println(Vr);
        stream.print(F("Serial Number: ")); stream.println(SN);
        stream.print(F("Device Address: ")); stream.println(DA);
    }
};

#endif // SUNSPEC_MODEL_1_H
