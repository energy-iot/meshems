// IFTTTAlerts.h
#pragma once

#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DTMPowerCache.h"
#include "Config.h"
/*
WARNING This is an in-progress initial draft implementation to support lite-weight  framework of local data-model driven IFTTT alerts and automations to decouple decision based policy enforcements 

TODO an example set of alerts that can be done on local streetpoleEMS
areas for alerts are mapped to OPENAMI areas of subtopics such as
Subanel_ENV   
  alert for when door openend and door closed with duration
    alert for when temp exceeds 40c  cutionary alert panel is hot set watchdog if panel jumps another 5c in  1 hour
    alert for when cabinet temp exceeds 50c back off on any fully loaded tenant connections - wires could be heating up
Subpanel_RCMLeaks 
    alert for when leakage first appears on any phase - timestamp and start watchdog on next leakage ramp up/down report 
Subpanel Harmonics 
    alert for when harmonics grow beyond set threshold - timestamp and start watchdog on next harmonics ramp up/down report
Tenant meter report
  green alert when goes to no load
  yellow alert when ramps to max current over certain slope and duration
  orange  alert when load is sustained max over certain duration



*/
struct Alert {
  String type;
  String message;
};

class IFTTTAlerts {
public:
  IFTTTAlerts(PubSubClient& client);
  void begin();
  void evaluate(const std::map<String, Stats>& stats, const std::map<String, float>& totals);
  void publish();
  void setRate(unsigned long intervalMs);
  unsigned long getRate() const;
  bool readyToPublish();

private:
  PubSubClient& _mqtt;
  std::vector<Alert> _activeAlerts;
  unsigned long _lastPublishTime;
  unsigned long _publishInterval;

  void addIf(bool condition, const String& type, const String& message);
};

// IFTTTAlerts.cpp
#include "IFTTTAlerts.h"

IFTTTAlerts::IFTTTAlerts(PubSubClient& client) : _mqtt(client), _lastPublishTime(0), _publishInterval(60000) {}

void IFTTTAlerts::begin() {
  _activeAlerts.clear();
}

void IFTTTAlerts::evaluate(const std::map<String, Stats>& stats, const std::map<String, float>& totals) {
  _activeAlerts.clear();

  auto it = stats.find("voltage");
  if (it != stats.end()) {
    float v = it->second.mean();
    addIf(v < 215.0, "voltage", "Undervoltage detected: " + String(v, 1) + "V");
    addIf(v > 245.0, "voltage", "Overvoltage detected: " + String(v, 1) + "V");
  }

  it = stats.find("current");
  if (it != stats.end()) {
    float c = it->second.max;
    addIf(c > 20.0, "current", "Overcurrent detected: " + String(c, 1) + "A");
  }

  it = stats.find("power");
  if (it != stats.end()) {
    float p = it->second.mean();
    addIf(p < 10.0, "power", "Idle load detected: avg " + String(p, 1) + "W");
  }
}

void IFTTTAlerts::addIf(bool condition, const String& type, const String& message) {
  if (condition) {
    _activeAlerts.push_back({ type, message });
  }
}

void IFTTTAlerts::publish() {
  if (_activeAlerts.empty()) return;

  DynamicJsonDocument doc(1024);
  doc["device_id"] = "esp32s3-001";
  doc["timestamp"] = time(nullptr);
  JsonArray arr = doc.createNestedArray("alerts");

  for (const auto& a : _activeAlerts) {
    JsonObject obj = arr.createNestedObject();
    obj["type"] = a.type;
    obj["message"] = a.message;
  }

  char buffer[1024];
  size_t len = serializeJson(doc, buffer);
  _mqtt.publish((String(MQTT_TOPIC) + "/alerts").c_str(), buffer, len);
  _lastPublishTime = millis();
}

void IFTTTAlerts::setRate(unsigned long intervalMs) {
  _publishInterval = intervalMs;
}

unsigned long IFTTTAlerts::getRate() const {
  return _publishInterval;
}

bool IFTTTAlerts::readyToPublish() {
  return millis() - _lastPublishTime >= _publishInterval;
}
