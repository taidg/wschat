#include "connectionset.h"
#include "asioserver.h"

ConnectionSet::ConnectionSet() {

}

ConnectionSet::ConnectionSet(ConnectionSet &x) {
  websocketpp::lib::unique_lock<websocketpp::lib::mutex> lock(mutex_);
  connectionSet_ = x.connectionSet_;
}

void ConnectionSet::insert(websocketpp::connection_hdl hdl) {
  websocketpp::lib::unique_lock<websocketpp::lib::mutex> lock(mutex_);
  connectionSet_.insert(hdl);
}

void ConnectionSet::erase(websocketpp::connection_hdl hdl) {
  websocketpp::lib::unique_lock<websocketpp::lib::mutex> lock(mutex_);
  connectionSet_.erase(hdl);
}

void ConnectionSet::send(std::string message) {
  for (std::set<websocketpp::connection_hdl>::iterator iter =
         connectionSet_.begin();
       iter != connectionSet_.end(); iter++) {
    connection_ptr con =
      websocketpp::lib::static_pointer_cast<AsioServer::connection_type>
      (iter->lock());

    if (!con) {
      continue;
    }

    con->send(message, websocketpp::frame::opcode::TEXT);

  }
}

