/*
   -------------------------------------------------------------------
   EmonESP Serial to Emoncms gateway
   -------------------------------------------------------------------
   Adaptation of Chris Howells OpenEVSE ESP Wifi
   by Trystan Lea, Glyn Hudson, OpenEnergyMonitor

   Modified to use with the CircuitSetup.us Split Phase Energy Meter by jdeglavina
   Modified to use with EMS Workshop by dmendonca

   All adaptation GNU General Public License as below.

   -------------------------------------------------------------------

   This file is part of OpenEnergyMonitor.org project.
   EmonESP is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.
   EmonESP is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with EmonESP; see the file COPYING.  If not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "mqtt_client.h"
#include <TimeLib.h>
#include <WiFiMulti.h>
#include <data_model.h>
#include <config.h>
#include <ArduinoJson.h>
#include <modbus_master.h>
#include <sunspec_model_213.h>
#include <leakage_model_ivy41a.h>
//#include "modbus_devices.h" // added by Kevin - future use
#include "data_model.h"
#define ENABLE_DEBUG_MQTT = 1

WiFiClient transportClient;                 // the network client for MQTT (also works with EthernetLarge)
PubSubClient mqttclient(transportClient);   // the MQTT client

unsigned long mqtt_interval_ts = 0;
static char mqtt_data[128] = "";
static int mqtt_connection_error_count = 0;
String topic_device;          // StreetPoleEMS globally unique device topic (publish/subscribe under here)
String topic_cmd;             // command topic (for 'southbound' commands)

// Function prototype for mqtt_publish_json
void mqtt_publish_json(const char* subtopic, const JsonDocument* payload);

void generateTopics() {
  //the top-level device topic string, eg: OPENAMI_<streetpoleEMSid>
  topic_device = MQTT_TOPIC;
  topic_device.concat("/");
  topic_device.concat(getDeviceID());
  topic_device.concat("/");

  //the command topic we subscribe to, eg: OPENAMI_ECAE3D98/cmd
  
  topic_cmd = topic_device;
  topic_cmd.concat("cmd");
}

// -------------------------------------------------------------------
// MQTT Connect
// Called only when MQTT server field is populated
// -------------------------------------------------------------------
boolean mqtt_connect()
{
  Serial.printf("MQTT Connecting...timeout in:%d\r\n", transportClient.getTimeout());
  // todo ENABLE_DEBUG_MQTT=1;
  if (transportClient.connect(MQTT_SERVER, 1883) != 1) //8883 for TLS
  {
     Serial.println("MQTT connect timeout.");
     // todo ENABLE_DEBUG_MQTT=0;
     return (0);
  }

  //transportClient.setTimeout(60);//(MQTT_TIMEOUT);
  mqttclient.setSocketTimeout(6);//MQTT_TIMEOUT);
  mqttclient.setBufferSize(MAX_DATA_LEN + 200);
  mqttclient.setKeepAlive(180);

  if (strcmp(MQTT_USER, "") == 0) {
    //allows for anonymous connection
    mqttclient.connect(getDeviceID()); // Attempt to connect
  } else {
    mqttclient.connect(getDeviceID(), MQTT_USER, MQTT_PW); // Attempt to connect
  }

  if (mqttclient.state() == 0) {
    Serial.printf("MQTT connected: %s\r\n", MQTT_SERVER);
    
    //subscribe to command topic
    if (!mqttclient.subscribe(topic_cmd.c_str())) {
      delay(250);
      if (!mqttclient.subscribe(topic_cmd.c_str())) {
        delay(500);
        if (!mqttclient.subscribe(topic_cmd.c_str())) {
          Serial.printf("MQTT: FAILED TO SUBSCRIBE TO COMMAND TOPIC: %s\r\n", topic_cmd.c_str());
          return false;
        }
      }
    }
    Serial.printf("MQTT: SUBSCRIBED TO COMMAND TOPIC: %s\r\n", topic_cmd.c_str());
    //  mqttclient.publish(getDeviceTopic().c_str(), "connected"); // Once connected, publish an announcement..
  } else {
    Serial.println("MQTT failed: ");
    Serial.println(mqttclient.state());
    return (0);
  }
  return (1);
}

void mqtt_publish_StreetPoleEMS(String EMSId, const PowerData& meterData) {
  SunSpecModel213 sunSpecData;
  // TODO publish Model 213 here
  // For now assume phase A. This can be extended to put the meter readings in the
  // correct phase using configuration data about which meter is on which phase.
  sunSpecData.PhVphA = meterData.voltage;
  sunSpecData.AphA = meterData.current;
  sunSpecData.WphA = meterData.active_power * 1000;
  sunSpecData.TotWhImport = meterData.import_energy * 1000;
  sunSpecData.TotWhExport = meterData.export_energy * 1000;
  sunSpecData.Hz = meterData.frequency;
  sunSpecData.PFphA = meterData.power_factor;
  sunSpecData.VarphA = meterData.reactive_power * 1000;

  long timestamp = meterData.timestamp_last_report;
  String topicBuf = "subpanel_3Ph";

  JsonDocument jsonDoc;
  sunSpecData.toJson(jsonDoc);
  jsonDoc["timestamp"] = timestamp;

  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}

void mqtt_publish_Meter(String meterId, const PowerData& meterData) {
  SunSpecModel213 sunSpecData;
  // TODO publish Model 11 here
  // For now assume phase A. This can be extended to put the meter readings in the
  // correct phase using configuration data about which meter is on which phase.
  sunSpecData.PhVphA = meterData.voltage;
  sunSpecData.AphA = meterData.current;
  sunSpecData.WphA = meterData.active_power * 1000;
  sunSpecData.TotWhImport = meterData.import_energy * 1000;
  sunSpecData.TotWhExport = meterData.export_energy * 1000;
  sunSpecData.Hz = meterData.frequency;
  sunSpecData.PFphA = meterData.power_factor;
  sunSpecData.VarphA = meterData.reactive_power * 1000;

  long timestamp = meterData.timestamp_last_report;
  String topicBuf = "meter_";
  topicBuf.concat(meterId);

  JsonDocument jsonDoc;
  sunSpecData.toJson(jsonDoc);
  jsonDoc["timestamp"] = timestamp;

  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}

void mqtt_publish_Leakage(String meterId, const PowerData& meterData) {
  LeakageModel leakageData;

  // TODO prepare actionable DC and AC leakage measurments, patterns and stats, and faults, and outages
  // TODO add adaptive publish rate as leakage grows from none to 
  // TODO see modbus register suite from IVY Metering RCD, RVD, differentiator for AC leakage is to include phase angle of leakage current vs phase 
  // leakage can be powerflow direction sensitive  and dependant
  long timestamp = meterData.timestamp_last_report;
  String topicBuf = "leakage"; //TODO + phaseId; and/OR + meterid;

  JsonDocument jsonDoc;
  leakageData.toJson(jsonDoc);
  jsonDoc["timestamp"] = timestamp;

  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}


void mqtt_publish_json(const char* subtopic, const JsonDocument * payload) {
    String topicBuf;
    String jsonString;
    if (measureJson(*payload) >= 1024) {
      Serial.println("MQTT publish: payload too large");
      return;
    }
    serializeJson(*payload, jsonString);
    // It's annoying to have to set this limit, but maybe a static size is better for performance?
    char data[1024];
    jsonString.toCharArray(data, sizeof(data));
    topicBuf = topic_device;
    topicBuf.concat(subtopic);
    if (!mqttclient.publish(topicBuf.c_str(), data)) {
        Serial.println("MQTT publish: failed");
    }
#ifdef ENABLE_DEBUG_MQTT
    Serial.printf("topic: %s, data: %s\n", topicBuf.c_str(), data);
#endif
}

//pull apart a comma-sep colon-delim name:value string and publish the name:value pairs under 'subtopic'
void mqtt_publish_comma_sep_colon_delim(const char* subtopic, const char * data) {
    String topicBuf;
    char buf[256];
    Serial.printf("MQTT publish: size:%d chars", strlen(data));
    do {
      int pos = strcspn(data, ":");
      strncpy(buf, data, pos);
      buf[pos] = 0;
      String st(subtopic);
      topicBuf = topic_device;
      topicBuf.concat(st+"/");
      topicBuf.concat(buf);
      //topic_ptr[pos] = 0;
      data += pos;
      if (*data++ == 0) {
        break;
      }

      pos = strcspn(data, ",");
      strncpy(mqtt_data, data, pos);
      mqtt_data[pos] = 0;
      data += pos;

      if (!mqttclient.publish(topicBuf.c_str(), mqtt_data)) {
       Serial.println("MQTT publish: failed");
      }
#ifdef ENABLE_DEBUG_MQTT
      Serial.printf("topic: %s, data: %s\n", topicBuf.c_str(), mqtt_data);
#endif
    } while (*data++ != 0);
}

// Subscriber callback
//
// We're subscribed to the following topics:
// <top>/<device_id>/cmd
//
// 
void subscriber_callback(char* topic, uint8_t* payload, unsigned int length) {
  //sanity
  if (length > 254) {
    Serial.printf("MQTT CALLBACK: not handled: payload len overrun:%d\n", length);
    return;
  }
  if (strcmp(topic, topic_cmd.c_str()) == 0) {
    char payload_buf[length+1] = {0};
    strncpy(payload_buf, (char*)payload, length);
    payload_buf[length] = '\0'; //ensure null-termination
    Serial.printf("\n***MQTT CALLBACK: topic '%s', payload '%s'\n", topic, payload_buf);
    if (strstr(payload_buf, "report") == 0) {
      //trigger a data-model dump
      return;
    }
    if (strstr((char*)payload_buf, "meter") == 0) {
      //control the meter
      return;
    }
    if (strstr((char*)payload_buf, "bms") == 0) {
      //BMS command
      return;
    }
    if (strstr((char*)payload_buf, "inverter") == 0) {
      //inverter command
      return;
    }
    // add new commands here
  }
}

void setup_mqtt_client() {
  generateTopics();
  mqttclient.setCallback(subscriber_callback);
  if (!mqtt_connect()) {
    delay(250);
    if (!mqtt_connect()) {
      delay(500);
      if (!mqtt_connect()) {
        Serial.println("MQTT: FAILED TO CONNECT");
        return;
      }
    }
  }
  mqtt_interval_ts = now();
}

void loop_mqtt() {

      bool mqtt_connected = mqttclient.connected();
      if (!mqtt_connected) {
        mqtt_connected = mqtt_connect();
      }
      //mqtt_publish(input);
      if (mqtt_connected) {  
      // TODO not all telemetry has to publish on same iteration, different rates of publish , including adaptive meaningful rate is a good thing
       // TODO publish streetpoleEMS actionable telemetry
       // mqtt_publish_EMS("", last_reading);
       // Serial.println("Publishing EMS device stats!");
       // TODO publish per phase per streetpoleEMS actionable telemetry 
       // mqtt_publish_phase("", last_reading);
       // Serial.println("Publishing Phase stats!");

        //TODO publish 3 phase OPENAMI per meter/tenant energy totals per phase ;
      mqtt_publish_StreetPoleEMS("", readings[0]);
       Serial.println("Publishing ems");
       mqtt_publish_Leakage("", readings[0]);
       Serial.println("Publishing leakage");
          
        for(int i=0;i<MODBUS_NUM_METERS;i++) {
         
          mqtt_publish_Meter(String(i), readings[i]);  // TODO add modbus node number in the readings powerdata
          Serial.println("Publishing meter");
        // mqtt_publish_phase("", last_reading);
        // Serial.println("Publishing Phase stats!");
        // mqtt_publish_EMS("", last_reading);
        // Serial.println("Publishing EMS device stats!");
        /*char topicId[8];
        snprintf(topicId, sizeof(topicId), "%d", i);
        mqtt_publish_Meter(topicId, readings[i]);
        */
        }
      } else {
        Serial.println("MQTT not connected!");
      }
      mqtt_interval_ts = millis();
    mqttclient.loop();
}

void mqtt_restart()
{
  if (mqttclient.connected()) {
    mqttclient.disconnect();
  }
}

boolean mqtt_connected()
{
  return mqttclient.connected();
}

void mqtt_publish_door_opened() {
  char buf[32] = {0};
  sprintf(buf,"%s/door", topic_device);
  mqttclient.publish(buf, "open", 0);
}

void mqtt_publish_door_closed() {
  char buf[32] = {0};
  sprintf(buf,"%s/door", topic_device);
  mqttclient.publish(buf, "closed", 0);
}