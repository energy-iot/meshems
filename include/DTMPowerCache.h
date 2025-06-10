// DTMPowerCache.h
#pragma once

#include <map>
#include <ArduinoJson.h>
#include <limits>
#include <cmath>

struct Stats {
  float min = std::numeric_limits<float>::max();
  float max = std::numeric_limits<float>::lowest();
  float sum = 0;
  float sumSq = 0;
  int count = 0;

  void add(float val) {
    if (!isnan(val)) {
      min = fmin(min, val);
      max = fmax(max, val);
      sum += val;
      sumSq += val * val;
      count++;
    }
  }

  float mean() const { return (count > 0) ? sum / count : 0; }
  float variance() const { return (count > 1) ? (sumSq - (sum * sum) / count) / (count - 1) : 0; }

  void reset() {
    min = std::numeric_limits<float>::max();
    max = std::numeric_limits<float>::lowest();
    sum = 0;
    sumSq = 0;
    count = 0;
  }
};

class DTMPowerCache {
public:
  void init();
  void addSamples(const std::map<String, String>& raw);
  DynamicJsonDocument buildJson();
  void resetStats();

  const std::map<String, Stats>& getStats() const { return statsMap; }
  const std::map<String, float>& getTotals() const { return totalizers; }

private:
  std::map<String, Stats> statsMap;
  std::map<String, float> totalizers;
};

// DTMPowerCache.cpp
//#include "DTMPowerCache.h"

// Define which registers are totalizers
static const std::set<String> totalizerRegs = {
  "36", "37", "38", "39" // Example: last 4 registers
};

void DTMPowerCache::init() {
  statsMap.clear();
  totalizers.clear();
}

void DTMPowerCache::addSamples(const std::map<String, String>& raw) {
  for (const auto& [key, valStr] : raw) {
    float val = valStr.toFloat();
    if (totalizerRegs.count(key)) {
      totalizers[key] = val;
    } else {
      statsMap[key].add(val);
    }
  }
}

DynamicJsonDocument DTMPowerCache::buildJson() {
  DynamicJsonDocument doc(2048);
  JsonObject root = doc.to<JsonObject>();
  root["device_id"] = "esp32s3-001";
  root["timestamp"] = time(nullptr);
  JsonObject regs = root.createNestedObject("registers");

  for (const auto& [key, stat] : statsMap) {
    JsonObject obj = regs.createNestedObject(key);
    obj["min"] = stat.min;
    obj["max"] = stat.max;
    obj["mean"] = stat.mean();
    obj["variance"] = stat.variance();
  }

  for (const auto& [key, val] : totalizers) {
    regs[key]["value"] = val;
  }

  return doc;
}

void DTMPowerCache::resetStats() {
  for (auto& [_, stat] : statsMap) {
    stat.reset();
  }
}
