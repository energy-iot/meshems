#include "mqtt_client.h"
#include <TimeLib.h>
#include <WiFiMulti.h>
#include <data_model.h>

#define mqtt_publish_interval 30000
String mqtt_server("192.168.1.70");
String mqtt_user;
String mqtt_pass;

String DEVICE_ID;
WiFiClient transportClient;                 // Create Wifi client for MQTT
PubSubClient mqttclient(transportClient);   // Create client for MQTT

static char input[MAX_DATA_LEN];
unsigned long mqtt_interval_ts = 0;

long lastMqttReconnectAttempt = 0;
int clientTimeout = 0;
int i = 0;

static char mqtt_device_id[32] = "";
static char mqtt_data[128] = "";
static int mqtt_connection_error_count = 0;
String cmd_topic_buf;
String device_topic_buf;
String device_resource_topic_buf;

void generateTopics() {
  device_topic_buf = "NESL";
  device_topic_buf.concat("/");
  device_topic_buf.concat(DEVICE_ID.c_str());
  device_topic_buf.concat("/");

  cmd_topic_buf = device_topic_buf;
  cmd_topic_buf.concat("cmd");

  device_resource_topic_buf = device_topic_buf;
  device_resource_topic_buf.concat("res");

}

// -------------------------------------------------------------------
// MQTT Connect
// Called only when MQTT server field is populated
// -------------------------------------------------------------------
boolean mqtt_connect()
{
  Serial.printf("MQTT Connecting...timeout in:%d\r\n", transportClient.getTimeout());

  if (transportClient.connect(mqtt_server.c_str(), 1883) != 1) //8883 for TLS
  {
     Serial.println("MQTT connect timeout.");
     return (0);
  }

  //transportClient.setTimeout(60);//(MQTT_TIMEOUT);
  mqttclient.setSocketTimeout(6);//MQTT_TIMEOUT);
  mqttclient.setBufferSize(MAX_DATA_LEN + 200);
  mqttclient.setKeepAlive(180);

  if (mqtt_user.length() == 0) {
    //allows for anonymous connection
    mqttclient.connect(DEVICE_ID.c_str()); // Attempt to connect
  } else {
    mqttclient.connect(DEVICE_ID.c_str(), mqtt_user.c_str(), mqtt_pass.c_str()); // Attempt to connect
  }

  if (mqttclient.state() == 0)
  {
    Serial.printf("MQTT connected: %s\r\n", mqtt_server.c_str());
    
    //subscribe to command topic
    if (!mqttclient.subscribe(cmd_topic_buf.c_str())) {
      delay(250);
      if (!mqttclient.subscribe(cmd_topic_buf.c_str())) {
        delay(500);
        if (!mqttclient.subscribe(cmd_topic_buf.c_str())) {
          Serial.printf("MQTT: FAILED TO SUBSCRIBE TO COMMAND TOPIC: %s\r\n", cmd_topic_buf.c_str());
          return false;
        }
      }
    }
    Serial.printf("MQTT: SUBSCRIBED TO COMMAND TOPIC: %s\r\n", cmd_topic_buf.c_str());
    //  mqttclient.publish(getDeviceTopic().c_str(), "connected"); // Once connected, publish an announcement..
  } else {
    Serial.println("MQTT failed: ");
    Serial.println(mqttclient.state());
    return (0);
  }
  return (1);
}

//construct a comma-sep colon-delim string for the publish method
void mqtt_publish_meter() {
  char buf[64];
  sprintf(buf,"temp:%2.1f,V1:%2.2f,V2:%2.2f,freq:%2.1f", (float)inputRegisters[0], (float)inputRegisters[1], (float)inputRegisters[2], (float)inputRegisters[3]);
  mqtt_publish_comma_sep_colon_delim("meter", buf);  
}

void mqtt_publish_comma_sep_colon_delim(const char* subtopic, const char * data) {
    String topicBuf;
    char buf[256];
    Serial.printf("MQTT publish: size:%d chars", strlen(data));
    do {
      int pos = strcspn(data, ":");
      strncpy(buf, data, pos);
      buf[pos] = 0;
      String st(subtopic);
      topicBuf = device_topic_buf;
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

void subscriber_callback(char* topic, uint8_t* payload, unsigned int length) {
  //sanity
  if (length > 254) {
    Serial.printf("MQTT CALLBACK: not handled: payload len overrun:%d\n", length);
    return;
  }
  if (strcmp(topic, cmd_topic_buf.c_str()) == 0) {
    char payload_buf[length+1] = {0};
    strncpy(payload_buf, (char*)payload, length);
    payload_buf[length] = '\0'; //ensure null-termination
    Serial.printf("\n***MQTT CALLBACK: topic '%s', payload '%s'\n", topic, payload_buf);
    //"report" command triggers an EVSE data model dump to MQTT
    if (strncmp(payload_buf, "report", 6) == 0) {
      return;
    }
    if (strncmp((char*)payload, "meter/", 5) == 0) {
      return;
    }
    if (strncmp((char*)payload, "battery/", 5) == 0) {
      return;
    }
    // add new commands here
  }
}

void generateDeviceID() {
  uint32_t low = ESP.getEfuseMac() & 0xFFFFFFFF;
  uint32_t high = (ESP.getEfuseMac() >> 32) % 0xFFFFFFFF;
  uint64_t fullMAC = word(low, high);
  char _id[32];
  sprintf(_id, "NESL%X%X", high, low);
  DEVICE_ID = _id;
}

const char* getDeviceID() {
  return DEVICE_ID.c_str();
}

void setup_mqtt_client() {
  generateDeviceID(); //must be called first
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

    if ((millis() - mqtt_interval_ts > mqtt_publish_interval)) {
      bool mqtt_connected = mqttclient.connected();
      if (!mqtt_connected) {
        mqtt_connected = mqtt_connect();
      }
      //mqtt_publish(input);
      if (mqtt_connected) {
        mqtt_publish_meter();
      }
      mqtt_interval_ts = millis();
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

void mqtt_publish_door_opened() {
  char buf[40] = {0};
  sprintf(buf,"%s/door", device_resource_topic_buf);
  mqttclient.publish(buf, "open", 0);
}

void mqtt_publish_door_closed() {
  char buf[40] = {0};
  sprintf(buf,"%s/door", device_resource_topic_buf);
  mqttclient.publish(buf, "closed", 0);
}