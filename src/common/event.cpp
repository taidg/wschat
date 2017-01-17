#include "event.h"

Event EventQueue::waitForEvent() {
  Event event;
  websocketpp::lib::unique_lock<websocketpp::lib::mutex> lock(mutex_);

  while (eventQueue_.empty()) {
    cond_.wait(lock);
  }

  event = eventQueue_.front();
  eventQueue_.pop();
  return event;
}

void EventQueue::push(Event event) {
  websocketpp::lib::unique_lock<websocketpp::lib::mutex> lock(mutex_);
  eventQueue_.push(event);
  cond_.notify_one();
}
