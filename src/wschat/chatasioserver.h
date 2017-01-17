#ifndef CHATASIOSERVER_H
#define CHATASIOSERVER_H

#include "asioserver.h"
#include "user.h"

struct custom_data {
  User *user;

  ~custom_data() {
    if (user != NULL) {
      delete user;
    }
  }
};

#endif // CHATASIOSERVER_H
