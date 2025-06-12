#include "DTMPowerCache.h"
#include <set>
#include <ctime>

static const std::set<String> totalizerRegs = { "36", "37", "38", "39" };

// -------------------- Stats methods --------------------
void Stats::add(float val) {
  if (!isnan(val)) {
    min = fmin(min, val);
    max = fmax(max, val);
    sum += val;
    sumSq += val * val;
    count++;
  }
}

float Stats::mean() const {
  return (count > 0) ? sum / count : 0;
}

float Stats::variance() const {
  return (count > 1) ? (sumSq - (sum * sum) / count) / (count - 1) : 0;
}

void Stats::reset() {
  min = std::numeric_limits<float>::max();
  max = std::numeric_limits<float>::lowest();
  sum = 0;
  sumSq = 0;
  count = 0;
}

// ------------------ DTMPowerCache methods ------------------

void DTMPowerCache::init() {
  statsMap.clear();
  totalizers.clear();
}

void DTMPowerCache::addSamples(const std::map<String, String>& raw) {
  for (const auto& pair : raw) {
    const String& key = pair.first;
    const String& valStr = pair.second;

    float val = valStr.toFloat();
    if (totalizerRegs.count(key)) {
      totalizers[key] = val;
    } else {
      statsMap[key].add(val);
    }
  }
}

DynamicJsonDocument DTMPowerCache::buildJson() {
  DynamicJsonDocument internalDoc(2048);  // Correct and supported

  JsonObject root = internalDoc.to<JsonObject>();
  root["device_id"] = "esp32s3-001";
  root["timestamp"] = static_cast<uint32_t>(time(nullptr));

  JsonObject regs = root["registers"].to<JsonObject>();

  for (const auto& pair : statsMap) {
    const String& key = pair.first;
    const Stats& stat = pair.second;

    JsonObject obj = regs[key].to<JsonObject>();
    obj["min"] = stat.min;
    obj["max"] = stat.max;
    obj["mean"] = stat.mean();
    obj["variance"] = stat.variance();
  }

  for (const auto& pair : totalizers) {
    const String& key = pair.first;
    float val = pair.second;

    regs[key]["value"] = val;
  }

  return internalDoc;
}

void DTMPowerCache::resetStats() {
  for (auto& pair : statsMap) {
    pair.second.reset();
  }
}
