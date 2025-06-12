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

  void add(float val);
  float mean() const;
  float variance() const;
  void reset();
};

class DTMPowerCache {
public:
  void init();
  void addSamples(const std::map<String, String>& raw);
  //JsonDocument buildJson();
  DynamicJsonDocument buildJson();
  void resetStats();

  const std::map<String, Stats>& getStats() const { return statsMap; }
  const std::map<String, float>& getTotals() const { return totalizers; }

private:
  std::map<String, Stats> statsMap;
  std::map<String, float> totalizers;
};