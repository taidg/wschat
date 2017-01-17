#include "user.h"

#include <cstring>

User::User(std::string username) : username_(username), timesIndex_(0) {
  memset(messageTimes_, 0, perMinuteMessageLimit * sizeof(time_t));
}

std::string User::getUsername() {
  return username_;
}

time_t User::timeTillNextAllowedMessage() {
  time_t allowedTime = messageTimes_[timesIndex_] + static_cast<time_t>(60);
  time_t now = time(NULL);
  return (allowedTime > now ? allowedTime - now : 0);
}

void User::updateMessageTimes() {
  messageTimes_[timesIndex_] = time(NULL);
  timesIndex_ = (timesIndex_ + 1) % perMinuteMessageLimit;
}
