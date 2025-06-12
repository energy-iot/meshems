 #pragma once

#include <modbus_master.h>
#include <data_model.h>

/**
 * @file modbus_chd130.h
 * @brief Modbus driver class for CHINT CHD130 single-phase energy meter
 *
 * This class implements a Modbus RTU interface for the CHINT CHD130 single-phase DIN rail energy meter.
 * It enables polling and caching of key energy and power quality metrics via RS-485.
 *
 * The CHD130 is commonly used in residential or commercial subpanels for tenant-level monitoring.
 * It supports a range of electrical parameters and may include DLMS/COSEM protocol support over an alternate interface
 * (for full IEC 62056-21 / OBIS code mapping), but this class only uses the Modbus RTU interface.
 *
 * Key Features Supported via Modbus:
 * - Voltage, current, frequency
 * - Active, reactive, and apparent power
 * - Power factor
 * - Total, import, and export energy (kWh)
 * - RS-485 Modbus RTU (9600 8N1 default)
 *
 * Optional:
 * - Integration with PowerPh1Data and MQTT telemetry frameworks
 * - External metadata access (serial number, firmware version, etc.) via extended registers
 *
 * TODO:
 * - Confirm register map and scaling factors from CHINT CHD130 technical documentation
 * - Extend class to support DLMS/COSEM OBIS code mapping if required in future firmware
 *
 * AUTHORS, GLENN ALGIE
 * CREATED     June 2025
 */
// TODO ADD APPARENT POWER SUPPORT Q: IS THAT REACTIVE POWER ?

class Modbus_CHD130 : public ModbusMaster {
    public:
        Modbus_CHD130();
        ~Modbus_CHD130() {};

        uint8_t get_modbus_address();
        void set_modbus_address(uint8_t addr);

        float read_modbus_value(uint16_t registerAddress);
        float read_modbus_extended_value(uint16_t registerAddress);

        enum MB_Reg {
            rVOLTAGE         = 0x0000,  // Register: Voltage (V)      (scale TBD)
            rCURRENT         = 0x0001,  // Register: Current (A)      (scale TBD)
            rACTIVE_POWER    = 0x0002,  // Register: Active Power (W) (scale TBD)
            //rAPPARENT_POWER  = 0x0003,  // Register: Apparent Power   (VA)
            rREACTIVE_POWER  = 0x0004,  // Register: Reactive Power   (Var)
            rPOWER_FACTOR    = 0x0005,  // Register: Power Factor
            rFREQUENCY       = 0x0006,  // Register: Frequency (Hz)
            rTOTAL_ENERGY    = 0x0100,  // Register: Total Energy (kWh) (scale TBD)
            rIMPORT_ENERGY   = 0x0102,  // Register: Import Energy
            rEXPORT_ENERGY   = 0x0104,  // Register: Export Energy
            rMETADATA        = 0x0200   // Optional device metadata
        };

        void poll();
        PowerData last_reading;

        float getTotalEnergy();
        float getImportEnergy();
        float getExportEnergy();
        float getVoltage();
        float getCurrent();
        float getActivePower();
        //float getApparentPower();
        float getReactivePower();
        float getPowerFactor();
        float getFrequency();

    private:
        uint8_t modbus_address;
        unsigned long timestamp_last_report;
        unsigned long timestamp_last_failure;

        float voltage;
        float current;
        float active_power;
        //float apparent_power;
        float reactive_power;
        float power_factor;
        float frequency;
        float total_energy;
        float import_energy;
        float export_energy;
};

extern Modbus_CHD130 chd130_1;
extern Modbus_CHD130 chd130_2;
extern Modbus_CHD130 chd130_3;