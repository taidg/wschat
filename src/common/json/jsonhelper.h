#ifndef JSONHELPER_H
#define JSONHELPER_H

#include "json/json.h"

namespace json {

void init();

std::string write(Json::Value val);

Json::Value parse(std::string str);
}


#endif // JSONHELPER_H

