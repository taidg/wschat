#ifndef CONNECTIONSET_H
#define CONNECTIONSET_H
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <set>

using websocketpp::connection_hdl;

class ConnectionSet {
 public:
  ConnectionSet();
  ConnectionSet(ConnectionSet &x);
  void insert(websocketpp::connection_hdl hdl);
  void erase(websocketpp::connection_hdl hdl);
  void send(std::string message);
 private:
  std::set<connection_hdl, std::owner_less<connection_hdl>> connectionSet_;
  websocketpp::lib::mutex mutex_;
};

#endif // CONNECTIONSET_H
