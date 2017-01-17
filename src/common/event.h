#ifndef EVENT_H
#define EVENT_H
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/common/thread.hpp>

#include "json/jsonhelper.h"

#include <queue>

class User;

struct Event {
  websocketpp::connection_hdl hdl;
  Json::Value jsonMessage;
};

class EventQueue {
  public:
    Event waitForEvent();
    void push(Event event);
  private:
    std::queue<Event> eventQueue_;
    websocketpp::lib::mutex mutex_;
    websocketpp::lib::condition_variable cond_;
};

#endif
