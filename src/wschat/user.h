#ifndef USER_H
#define USER_H

#include <string>
#include <ctime>

const int perMinuteMessageLimit = 10;

class User {
 public:
  User(std::string username_);
  std::string getUsername();
  time_t timeTillNextAllowedMessage();
  void updateMessageTimes();

 private:
  std::string username_;
  time_t messageTimes_[perMinuteMessageLimit];
  int timesIndex_;
};

#endif // USER_H
