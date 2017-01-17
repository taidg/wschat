#include "jsonhelper.h"

namespace {
Json::FastWriter writer;
Json::Reader reader;
}

void json::init() {
  writer.omitEndingLineFeed();
}

std::string json::write(Json::Value val) {
  return writer.write(val);
}

Json::Value json::parse(std::string str) {
  Json::Value value = Json::objectValue;
  if (reader.parse(str, value)) {
  return value;
    } else {
      value["status"] = "fail";
      value["reason"] = "invalid json";
      return value;
    }
}
