/*
   -------------------------------------------------------------------
   EmonESP Serial to Emoncms gateway
   -------------------------------------------------------------------
   Adaptation of Chris Howells OpenEVSE ESP Wifi
   by Trystan Lea, Glyn Hudson, OpenEnergyMonitor

   Modified to use with the CircuitSetup.us Split Phase Energy Meter by jdeglavina
   Modified to use with EMS Workshop by dmendonca
   Modified to use with openami over mqtt by galgie - flexible topic and subtopic reporting of 
        front-of-the-meter StreetPoleEMS and behind-the-meter MDU Building multiEV charge/discharge subpanels
        generate, store, consume, transform, transport actionable edge telemetry for use by both 
        distributed GroupLead EMS  and LVfeeder Lead EMS policy decision serving services.

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

Behind the meter Use cases:
    MDU BUILDINg MULTI TENANT BUSS mains LeadEMS publishes Mains to Wan cloud and to Building Lan as mqtt energy status source of truth. 
    other building subsystem EMS subpanels keep their subsystems in profile of Group Lead EMS distributing energy asset transfer policy
    schedules to the various building enrgy subsystems. At site install/staging time the EMS publish to each other on a well known mqtt discovery channel
    UPnP like  discovery of Building edges check-in with Mains lead EMS to receive each of their policy schedule bulk and periodically iterated updates

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
#include <leakage_model_ivy41a.h>         // these are actioanble leakage sensor measurements based on Type B leakage
//#include "modbus_devices.h"             // added by Kevin - future use
#include "data_model.h"

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
  // allow for 1883 or 8883 encrypted telemetry and command and control
  if (transportClient.connect(MQTT_SERVER, 1883) != 1) //8883 for TLS
  {
     Serial.println("MQTT connect timeout.");
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

void mqtt_publish_StreetPoleEMS(String EMSId, const PowerData& meterData) {  // TODO pass EMSdata structured model
  SunSpecModel213 sunSpecData;

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

void mqtt_publish_Meter(String meterId, const PowerData& meterData) { 
  SunSpecModel213 sunSpecData;

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

void mqtt_publish_Harmonics(const SunSpecModel213Harmonics& harmonicsData, long timestamp) { // TODO pass EMSdata structured model
  String topicBuf = "harmonics"; // Subtopic under the device topic

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

void loop_mqtt(PowerData last_reading) {  // TODO powerdata need have object instances per subtopic json document blobs 3Phase EMS, Harmonics, Leakage, 1 Phase meters

      bool mqtt_connected = mqttclient.connected();
      if (!mqtt_connected) {
        mqtt_connected = mqtt_connect();
      }
      //mqtt_publish(input);
      if (mqtt_connected) {  
      // TODO not all telemetry has to publish on same iteration, different rates of publish , including adaptive meaningful rate is a good thing


        //TODO publish 3 phase OPENAMI per meter/tenant energy totals per phase ;
        mqtt_publish_StreetPoleEMS("", last_reading);
        Serial.println("Publishing actionable ems 3phase telemetry!");
        //mqtt_publish_Harmonics(HarmonicsData, last_reading.timestamp_last_report);
        //TODO Serial.println("Publishing actionable harmonics telemetry!");
        mqtt_publish_Leakage("", last_reading);
        Serial.println("Publishing actionable leakage telemetry!");
        mqtt_publish_Meter("meterid", last_reading);
        Serial.println("Publishing actionable meter/tenant telemetry!");

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