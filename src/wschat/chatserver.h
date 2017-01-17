#include <map>
#include <set>

#include "chatasioserver.h"
#include "json/jsonhelper.h"

typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> ConnectionSet;
using websocketpp::connection_hdl;

// Relays Chat messages between clients. Simple usage:
//    ChatServer c;
//    c.run();
class ChatServer {
 public:
  ChatServer();
  void openHandler(websocketpp::connection_hdl hdl);
  void closeHandler(websocketpp::connection_hdl hdl);
  void postAuthCloseHandler(websocketpp::connection_hdl hdl);
  void preAuthMessageHandler(websocketpp::connection_hdl hdl,
                             AsioServer::message_ptr msg);
  void postAuthMessageHandler(websocketpp::connection_hdl hdl,
                              AsioServer::message_ptr msg);
  void processMessages();
  void run();
  void log(std::string message);

  void setDhparamFile(const std::string &dhparamFile);
  void setPrivateKeyFile(const std::string &privateKeyFile);

private:
  void close(websocketpp::connection_hdl hdl, std::string reason);

  std::string privateKeyFile_;
  std::string dhparamFile_;
  AsioServer server_;

  std::queue<std::pair<websocketpp::connection_hdl, Json::Value> > messageQueue_;

  std::map<std::string, websocketpp::connection_hdl> userToConnectionMap_;
  std::map<connection_hdl, User, std::owner_less<connection_hdl>> connectionToUserMap_;
  ConnectionSet newConnections_;

  std::map<std::string, ConnectionSet> channelMap_;

  websocketpp::lib::mutex messagesMutex_;
  websocketpp::lib::mutex userMutex;
  websocketpp::lib::mutex connectionsMutex_;
  websocketpp::lib::mutex channelMutex_;

  websocketpp::lib::condition_variable  connectionsCond_;
  websocketpp::lib::condition_variable  messagesCond_;

  ConnectionSet meadow_;
  Json::FastWriter jsonWriter_;
  Json::Reader jsonReader_;
};

