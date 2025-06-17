/*
   -------------------------------------------------------------------
   EmonESP Serial to Emoncms gateway
   -------------------------------------------------------------------
   Adaptation of Chris Howells OpenEVSE ESP Wifi
   by Trystan Lea, Glyn Hudson, OpenEnergyMonitor

   Modified to use with the CircuitSetup.us Split Phase Energy Meter by jdeglavina
   Modified to use with EMS Workshop by dmendonca
   Modified to use with draft openami over mqtt by galgie - flexible topic and subtopic reporting of 
        front-of-the-meter StreetPoleEMS and behind-the-meter MDU Building multiEV charge/discharge subpanels
        mediate the resal time energy transfer functions of generate, store, consume, transform, transport actionable edge telemetry for use by both 
        introduced the concept of a distributed LeadEMS  and LVfeeder Lead EMS policy decision serving node
        located within the village, implemented as a Linux Java aggregation node planned for every 50-100 streetpoleEMS and may reside adjacent to a legacy STS/DLMS DCU node. 

   All adaptation GNU General Public License as below.

   -------------------------------------------------------------------

   This file is part of OpenEnergyMonitor.org project.  
   It was further extended by NESL.energy for additional front and behind the meter Sunspec Model report format introducing OPENAMI energy reporting 
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

Behind the meter Use cases:
    MDU BUILDINg MULTI TENANT BUSS main a LeadEMSs at the MAINs publishes MAINs and balanced energy subsystems to Wan cloud and to Building Lan as mqtt energy status source of truth. 
    other building subsystem EMS subpanels such as HVAC and MultiEV and/or GismoPower stalls or building kitchen appliance subpanels keep each of  their subsystems in profile of Group Lead EMS distributing energy asset transfer policy
    schedules to the various building enrgy subsystems.     
The subsystems EMSs publish to the LeadEMS and optionally to each other on a well known mqtt discovery and negotiation channel
    UPnP like  discovery of Building edges check-in with the designated MAINs LeadEMS to receive each of their policy schedule bulk and periodically iterated updates
    The Lead EMS is a dual ESP32S3 running mostly in hotstandby reporting and remote configurable on an 2 way mqtt OPENAMI to the Utiity cloud edge and i=on a transient as needed can dro sync and allow the standby ESP32 to host a web browser that
    shows energy utlitizations stats and can accepte authenticated local command UX , THe behind the meter architecture is designed as a no single point of failure.  a n:1 EMS sparing strategy is possible in the 
Front of the Meter Use Case
   IEEE ISV StreetPoleEMS multi tenant bidirectional energy asset transfer Policy Enforcer Edge at the Smartened Village StreetPole. S\Each StreetPoleENS communicates with distributed GroupLead EMS policy serving Java Linux Nodes.
   Thes lead Java Linux nodes receive periodic usage telemetry from l\discoverred StreetPoleEMS edges and from front of meter and behind the meter DERs
   that periodically advertise their capabilities and name plate and present utilized capacities - key dimensions of DERs capabilities are  geneerate, transform, store and consume. 
   THe multiple streetpoleems edges  on a shared LVfeeder also has a single (and backup) declared "LVfeeder Lead EMS" that keeps a totalizer data maodel source of truth of the LVfeeder 
   energy transport nameplate capacity and present capacity sharing this over mqtt on a well known  published/discovered mqtt too the GroupLead policcy serving EMS ( Java Linix multicore node. 
   The Java Linux multicore GroupLead EMS policy serving nodes are independant decision makers for one or more designated or learned LVFeeders that it serves the generate, store, transform, and consume time-of-day policies to 

TODO - soon is to breakup this mqtt client as its taking on mqtt higher level separated roles for a designated LVFeeder LeadEMS vs a regular policy enforcing building
   or streetpoleEMS edge of a building energy subsystem specifc policy enforcer or streetpoleEMS LVfeeder EMS policy enforcer role at multitenant subpanel
TODO perhaps can decouple the higher level topics telemetry formating of the key "openami" schema framework separate from the basic emchanical operation of 
establishing and encode and decode json documents and mqtt operations of a 2way mqtt monitor and control plane framework. key openami subtopics
   o  EMS-3phase energy reports actionable telemetry
   o  EMS-harmonics energy actionable telemetry
   o  EMS- leakage energy actioanable telemetry
   o  per tenant meter single phase energy reports with localized leakage actionable telemetry
   o  EMS meaningful stats - actionable OAM learning  telemetry
   o  Tenant per meter stats - for example time stats in active operational edge DER  roles of generate, store, consume, transform
   add ability to add or remove openami subtopics based on the subpanel SKU and onbaord addressable 
   edge sensors (meters, leakage RCM) and actuators (normally closed and normally open designated contactors).
   
   IN a energy equity village and villager  empowerment business model its important to keep a lean well defined 2way mqtt measure and comman and control
   "potential" operations of the edge tenant or streetpole site edge ability to perform in all 4 or 5 modes of addressable 
   session oriented energy asset transfer policy measureable and enforceable energy categories of:
   o consume
   o generate
   o store
   o transform
   o transport

   Ideally each meter should have stats published of its powerflow managed energy asset transfer individual sessions performing as a 
   generate(export), store, consume(import), transform (AC-DC voltage coupled form, voltage level conversion),  transport (as a LVFeeder Lead EMS operational role) managed energy servcice entities  
  */

#include "mqtt_client.h"
#include <TimeLib.h>
#include <WiFiMulti.h>
#include <data_model.h>
#include <config.h>
#include <ArduinoJson.h>
#include <modbus_master.h>
#include <sunspec_model_213.h>            // TODO breaks up into base and harmonics separated subtopics for openami
#include <sunspec_model_213_base.h>       // stays true to Sunspec base 213 data model schema
#include <sunspec_model_213_harmonics.h>  // TODO confirm if there is a harmonics report for Sunspec model and adapt or change to be flexible
#include <leakage_model_ivy41a.h>         // these are actioanable leakage sensor measurements based on Type B leakage
#include <sunspec_model_1.h> 
#include <sunspec_model_11.h>    
#include <ems_env_model.h>    
//#include "modbus_devices.h"             // added by Kevin - future use
#include "data_model.h"
#define ENABLE_DEBUG_MQTT = 1

// some mqtt banditch stats to include hourly/daily as its own publish
unsigned long mqtt_payload_bytes = 0;
unsigned long mqtt_tcpip_bytes = 0;
unsigned long mqtt_publish_count = 0;
unsigned long last_bandwidth_report_time = 0;
//const unsigned long BANDWIDTH_REPORT_INTERVAL_MS = 3600000; // 1 hour report interval on mqtt bandwidth stats per subpanel
const unsigned long BANDWIDTH_REPORT_INTERVAL_MS = 300000; // debug only 5 min report interval on mqtt bandwidth stats per subpanel


WiFiClient transportClient;                 // the network client for MQTT (also works with EthernetLarge)
PubSubClient mqttclient(transportClient);   // the MQTT client

// TODO is to allow build time control bools here to enable openami schema subtopics to be included or not based on a subpanel model - TBD

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
  // todo ENABLE_DEBUG_MQTT=1;  // allow for 1883 or 8883 encrypted telemetry and command and control
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

void mqtt_publish_EMS_3Ph(String EMSId, const PowerData& meterData) {  // TODO pass EMSdata structured model
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
  // String topicBuf = EMSId;

  JsonDocument jsonDoc;
  sunSpecData.toJson(jsonDoc);
  jsonDoc["timestamp"] = timestamp;

  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}

void mqtt_publish_Meter(int meterId, const PowerData& meterData) { 
  SunSpecModel11 sunSpecData;
  // publish Model 11  SUnspec schema here for per tenant single phase
  // For now assume phase A. This can be extended to put the meter readings in the
  // correct phase using configuration data about which meter is on which phase.
  //TODO grab the specific cached meterId  PowerData[i] for example
  sunSpecData.Phase= meterId; // assume 1 tenenat per phase in a 3 tenant 3ph subpanel , TODO part of stage operation and subpanel schema backed up  to a subpanel staging cloud 
  sunSpecData.PhV= meterData.voltage;
  sunSpecData.PhA = meterData.current;
  sunSpecData.PhW = meterData.active_power * 1000;
  sunSpecData.TotWhImport = meterData.import_energy * 1000;
  sunSpecData.TotWhExport = meterData.export_energy * 1000;
  sunSpecData.Hz = meterData.frequency;
  sunSpecData.PF = meterData.power_factor;
  sunSpecData.Var = meterData.reactive_power * 1000;
// TODO add actioanable locally interpreted metadata to the report 

  long timestamp = meterData.timestamp_last_report;
  String topicBuf = "meter_";
  topicBuf.concat(meterId);

  JsonDocument jsonDoc;
  sunSpecData.toJson(jsonDoc);
  jsonDoc["timestamp"] = timestamp;

  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}

void mqtt_publish_EMS_MFR(String EMSId, long timestamp) { // TODO pass EMSdata structured model
  String topicBuf = "subpanel_MFR"; // Subtopic under the device topic
  SunSpecModel1_EMS MFRData;
  JsonDocument jsonDoc;
  MFRData.toJson(jsonDoc);  // This assumes you have a method toJson() defined for harmonics
  jsonDoc["timestamp"] = timestamp;

  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}

void mqtt_publish_EMS_ENV(String EMSId, long timestamp) {
  String topicBuf = "subpanel_ENV"; // Subtopic under the device topic
  JsonDocument jsonDoc;
  EMS_ENV_Model EMS_ENV_cache;
  EMS_ENV_cache.toJson(jsonDoc);
  jsonDoc["timestamp"] = timestamp;
  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}
void mqtt_publish_Harmonics(String EMSId, long timestamp) { // TODO pass EMSdata structured model
  String topicBuf = "subpanel_harmonics"; // Subtopic under the streetPoleEMS per unique nodal topic
  SunSpecModel213Harmonics harmonicsData;
  JsonDocument jsonDoc;
  harmonicsData.toJson(jsonDoc);  // This assumes you have a method toJson() defined for harmonics
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
  String topicBuf = "subpanel_RCMleaks"; //TODO + phaseId; and/OR + meterid;

  JsonDocument jsonDoc;
  leakageData.toJson(jsonDoc);
  jsonDoc["timestamp"] = timestamp;

  mqtt_publish_json(topicBuf.c_str(), &jsonDoc);
}

/*
MQTT Stats
What We’ll Track and report hourly
  MQTT payload bytes (JSON body).
  Estimated TCP/IP overhead per publish (default assumption: ~60 bytes per publish).
  Packet count.
*/


void mqtt_publish_bandwidth_stats() {
    JsonDocument statsDoc;
    statsDoc["interval_ms"] = BANDWIDTH_REPORT_INTERVAL_MS;
    statsDoc["publish_count"] = mqtt_publish_count;
    statsDoc["payload_bytes"] = mqtt_payload_bytes;
    statsDoc["tcpip_bytes"] = mqtt_tcpip_bytes;
    statsDoc["timestamp"] = now();

    mqtt_publish_json("subpanel_stats/bandwidth", &statsDoc);

    // Reset counters
    mqtt_payload_bytes = 0;
    mqtt_tcpip_bytes = 0;
    mqtt_publish_count = 0;
}




/*
Json util and the mqtt PUBLISH main method

older method working without stats:
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
*/
void mqtt_publish_json(const char* subtopic, const JsonDocument* payload) {
    String topicBuf;
    String jsonString;

    size_t payload_len = measureJson(*payload);
    if (payload_len >= 1024) {
        Serial.println("MQTT publish: payload too large");
        return;
    }

    serializeJson(*payload, jsonString);
    char data[1024];
    jsonString.toCharArray(data, sizeof(data));
    
    topicBuf = topic_device + subtopic;

    if (!mqttclient.publish(topicBuf.c_str(), data)) {
        Serial.println("MQTT publish: failed");
    } else { // update stats on mqtt bandwidth used per streetpoleEMS
        mqtt_payload_bytes += payload_len;
        mqtt_tcpip_bytes += payload_len + 60; // TCP/IP+MQTT overhead
        mqtt_publish_count++;
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
  uint32_t loop_timestamp = esp_log_timestamp();

      bool mqtt_connected = mqttclient.connected();
      if (!mqtt_connected) {
        mqtt_connected = mqtt_connect();
      }
      //mqtt_publish(input);
      if (mqtt_connected) {  
      // TODO not all telemetry has to publish on same loop iteration, different rates of publish , 
      // target requirement is publish on a adaptive meaningful rate is a lean bandwidth on the meshed G3/PLC/Wireless/LORA MAC/PHY future iteration


        
       // First is to  publish Sunspec model 1 subpanel manufacture details TODO is 1 per day or per hour
      mqtt_publish_EMS_MFR("", loop_timestamp);  //publish Sunspec model 1 manufacture details
      Serial.println("Published EMS Model 1 MFR Info");

      // publish real time subpanel cabinet environmetals such as temp/pressure/humid/  tamper and 
      // TODO integrate shock and  loud noise and possily street pole image snapsot on demand from mqtt command
      mqtt_publish_EMS_ENV("", loop_timestamp);
      Serial.println("Published subpanel environmental data");
      
    // TODO Next publish Sunspec model xyz subpanel DER nameplate capacity  rating 
      // mqtt_publish_EMS_Rated("", readings[0]);  //publish Sunspec model xyza rating details
      // Serial.println("Published EMS nameplate");
       //TODO publish 1 or 3 phase OPENAMI per subpanel peer phase and per meter/tenant energy usage (TODO scope is consumed, generated, stored, transformed, distributed);
        //TODO  IF 3phase phase subpanel setup then - assume 3 phase subpanel 
      mqtt_publish_EMS_3Ph("", readings[0]);  // publish Sunspec model 213 schema for the 3 phase subpanel
      // TODO else publish single phase subpanel totalizer metrics
      //  mqtt_publish_EMS_1Ph("", readings[0]);  // publish Sunspec model 213 schema for the 3 phase subpanel
       Serial.println("Published EMS per Phase Totalizers");
       //next is publish EMS per phase Leakage , TODO adpative rate: if leakage is non zero or leakage fault or leakage changed
      mqtt_publish_Leakage("", readings[0]);
      Serial.println("Published per phase leakage");

      //next is publish EMS per phase Harmonics , TODO adpative rate: if leakage is non zero or leakage fault or leakage changed
      mqtt_publish_Harmonics("", loop_timestamp);
      Serial.println("Published per phase Harmonics");

    // next is loop over the subpanel per tenant meters   
    for(int i=0;i<MODBUS_NUM_METERS;i++) {
          mqtt_publish_Meter(i, readings[i]);  
          // TODO add modbus node number and per meter leakage RCD Fault in the readings powerdata
          Serial.println("Published tenant meter num:");
  
        /*char topicId[8];  // debug
        snprintf(topicId, sizeof(topicId), "%d", i);
        mqtt_publish_Meter(topicId, readings[i]);
        */
        }
      } else {
        Serial.println("MQTT not connected!");
      }
      mqtt_interval_ts = millis();
    if (millis() - last_bandwidth_report_time >= BANDWIDTH_REPORT_INTERVAL_MS) {
     mqtt_publish_bandwidth_stats();
     Serial.println("Published stats/bandwidth");
     last_bandwidth_report_time = millis();
    }
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


// TODO add door contact/tamper and publish in OPENAMI EMS nde subtopic
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