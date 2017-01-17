#include <cstdio>
#include <csignal>
#include <fstream>

#include <boost/program_options.hpp>

#include "chatserver.h"

using namespace websocketpp;
using namespace boost::program_options;

namespace arg = websocketpp::lib::placeholders;

const int kMaxChatMessageLength = 256;

ChatServer::ChatServer() {
  // Set logging level
  //server.clear_access_channels(log::alevel::all);
  //server.clear_error_channels(log::elevel::all);

  // Set up Json FastWriter
  jsonWriter_.omitEndingLineFeed();

  // Create default room.
  ConnectionSet meadow;
  channelMap_.insert(std::pair<std::string, ConnectionSet>("The Meadow", meadow));

  server_.set_open_handler(
    lib::bind(&ChatServer::openHandler, this, arg::_1));
  server_.set_close_handler(
    lib::bind(&ChatServer::closeHandler, this, arg::_1));
  server_.set_message_handler(
    lib::bind(&ChatServer::preAuthMessageHandler, this, arg::_1, arg::_2));
  server_.set_tls_init_handler(lib::bind(&on_tls_init, MOZILLA_INTERMEDIATE,
                                        arg::_1, "chatserver/dh.pem", "chatserver/server.pem"));

  server_.set_max_message_size(1000);
  server_.set_reuse_addr(true);

  server_.init_asio();
}

void ChatServer::openHandler(websocketpp::connection_hdl hdl) {
  connectionsMutex_.lock();
  newConnections_.insert(hdl);
  connectionsMutex_.unlock();

  websocketpp::lib::error_code errorCode;
  AsioServer::connection_ptr con = server_.get_con_from_hdl(hdl, errorCode);
  log("Connection: " +
      con->get_raw_socket().remote_endpoint().address().to_string());
}

void ChatServer::closeHandler(websocketpp::connection_hdl hdl) {
  connectionsMutex_.lock();
  newConnections_.erase(hdl);
  connectionsMutex_.unlock();
}

void ChatServer::postAuthCloseHandler(websocketpp::connection_hdl hdl) {
  // Remove from Maps
  userMutex.lock();
  std::string username = connectionToUserMap_.find(hdl)->second.getUsername();
  userToConnectionMap_.erase(username);
  connectionToUserMap_.erase(hdl);
  userMutex.unlock();

  //Remove from default room
  channelMutex_.lock();
  channelMap_.find("The Meadow")->second.erase(hdl);
  channelMutex_.unlock();
}

void ChatServer::preAuthMessageHandler(websocketpp::connection_hdl hdl,
                                       AsioServer::message_ptr msg) {
  // Determine message type
  Json::Value message;
  jsonReader_.parse(msg->get_payload(), message);

  std::string messageType = message["type"].asString();
  websocketpp::lib::error_code errorCode;

  // Quit if not auth request
  if (messageType.compare("auth") != 0) {
    Json::Value authError;
    authError["type"] = "error";
    authError["code"] = "auth_required";
    server_.send(hdl, jsonWriter_.write(authError),
                websocketpp::frame::opcode::TEXT, errorCode);
    return;
  }

  std::string username = message["username"].asString();

  userMutex.lock();
  bool userExists = userToConnectionMap_.find(username) !=
                    userToConnectionMap_.end();
  userMutex.unlock();

  // return error if username not set or is already taken
  if (username.empty()  || userExists) {
    Json::Value authError;
    authError["type"] = "error";
    authError["code"] = "auth_invalid";
    server_.send(hdl, jsonWriter_.write(authError),
                websocketpp::frame::opcode::TEXT, errorCode);
    return;
  }

  // Otherwise all is good. So add user to maps..
  userMutex.lock();
  userToConnectionMap_.insert(std::pair<std::string, connection_hdl>(username,
                             hdl));
  connectionToUserMap_.insert(std::pair<connection_hdl, User>(hdl, username));
  userMutex.unlock();

  // Remove user from set of new connections
  connectionsMutex_.lock();
  newConnections_.erase(hdl);
  connectionsMutex_.unlock();

  // Set new message handler and close handler
  AsioServer::connection_ptr con = server_.get_con_from_hdl(hdl, errorCode);
  con->set_message_handler(
    bind(&ChatServer::postAuthMessageHandler, this, arg::_1, arg::_2));
  con->set_close_handler(
    bind(&ChatServer::postAuthCloseHandler, this, arg::_1));

  // Put them in default room
  channelMutex_.lock();
  channelMap_.find("The Meadow")->second.insert(hdl);
  channelMutex_.unlock();

  // Return receipt
  Json::Value authReceipt;
  authReceipt["type"] = "auth";
  authReceipt["username"] = username;

  server_.send(hdl, jsonWriter_.write(authReceipt),
              websocketpp::frame::opcode::TEXT, errorCode);

  //log
  std::string address =
    con->get_raw_socket().remote_endpoint().address().to_string();
  char buffer[100];
  snprintf(buffer, 100, "%s logged in from %s", username.c_str(),
           address.c_str());
  log(buffer);

  return;
}

void ChatServer::postAuthMessageHandler(websocketpp::connection_hdl hdl,
                                        AsioServer::message_ptr msg) {
  // Determine message type
  Json::Value message;
  jsonReader_.parse(msg->get_payload(), message);
  messagesMutex_.lock();
  messageQueue_.push(std::pair<connection_hdl, Json::Value>(hdl, message));
  messagesMutex_.unlock();
  messagesCond_.notify_one();
}

void ChatServer::processMessages() {
  for /*ever*/ (;;) {
    lib::unique_lock<lib::mutex> messages_lock(messagesMutex_);

    while (messageQueue_.empty()) {
      messagesCond_.wait(messages_lock);
    }

    connection_hdl hdl = messageQueue_.front().first;
    Json::Value incomingMessage = messageQueue_.front().second;
    messageQueue_.pop();
    messages_lock.unlock();

    std::string messageType = incomingMessage["type"].asString();
    websocketpp::lib::error_code errorCode;

    if (messageType.compare("say") == 0) {
      userMutex.lock();
      User &user = connectionToUserMap_.find(hdl)->second;

      if (user.timeTillNextAllowedMessage() > 0) {
        userMutex.unlock();
        continue;
      }

      user.updateMessageTimes();
      std::string source = user.getUsername();
      userMutex.unlock();

      std::string text = incomingMessage["text"].asString();

      if (text.length() > kMaxChatMessageLength) {
        continue;
      }

      std::string targetName = incomingMessage["target"].asString();
      Json::Value outgoingMessage;
      outgoingMessage["type"] = "say";
      outgoingMessage["target"] = targetName;
      outgoingMessage["source"] = source;
      outgoingMessage["text"] = text;

      const std::string jsonMessage = jsonWriter_.write(outgoingMessage);

      channelMutex_.lock();
      ConnectionSet target = channelMap_.find(targetName)->second;
      channelMutex_.unlock();

      for (ConnectionSet::iterator iter = target.begin(); iter != target.end();
           iter++) {
        server_.send(*iter, jsonMessage, websocketpp::frame::opcode::TEXT, errorCode);
      }
    }
  }

  log("Process Thread exited. Should not happen.");
}

void ChatServer::run() {
  server_.set_open_handler(
    lib::bind(&ChatServer::openHandler, this, arg::_1));
  server_.set_close_handler(
    lib::bind(&ChatServer::closeHandler, this, arg::_1));
  server_.set_message_handler(
    lib::bind(&ChatServer::preAuthMessageHandler, this, arg::_1, arg::_2));
  server_.set_tls_init_handler(lib::bind(&on_tls_init, MOZILLA_INTERMEDIATE,
                                        arg::_1, dhparamFile_, privateKeyFile_));
  log("Server started");

  server_.listen(9002);
  server_.start_accept();

  lib::thread processMessageThread(&ChatServer::processMessages, this);

  log("Server started");
  server_.run();
}

void ChatServer::log(std::string message) {
  printf("%s\n", message.c_str());
  fflush(stdout);
}

void ChatServer::close(websocketpp::connection_hdl hdl, std::string reason) {
  //Create goodbye message
  Json::Value message;
  message["type"] = "goodbye";
  message["reason"] = reason;
  Json::FastWriter fastWriter;

  //close connection
  server_.close(hdl, websocketpp::close::status::going_away,
               fastWriter.write(message));
}
void ChatServer::setPrivateKeyFile(const std::string &privateKeyFile) {
  privateKeyFile_ = privateKeyFile;
}

void ChatServer::setDhparamFile(const std::string &dhparamFile) {
  dhparamFile_ = dhparamFile;
}


void SIGINT_handler(int param) {
  exit(1);
}

int main(int argc, char *argv[]) {
  try {
    std::string config_file;

    options_description generic("Generic options");
    generic.add_options()("config,c",
                          value<std::string>(&config_file)->default_value("config/config.cfg"));

    options_description config("Configuration");
    config.add_options()("pkey", value<std::string>())("dhparam",
        value<std::string>())("password", value<std::string>());

    variables_map vm;
    store(command_line_parser(argc, argv).options(generic).run(), vm);
    notify(vm);

    std::ifstream ifs(config_file.c_str());

    if (!ifs) {
      std::cout << "can not open config file: " << config_file << "\n";
      return 0;
    } else {
      store(parse_config_file(ifs, config), vm);
      notify(vm);
    }

    if (!vm.count("pkey")) {
      std::cout << "private key file required.\n";
      return 1;
    }

    if (!vm.count("dhparam")) {
      std::cout << "dh paramaters file required.\n";
      return 1;
    }

    // init json helper system
    json::init();

    signal(SIGINT, SIGINT_handler);
    ChatServer server;
    server.setDhparamFile(vm["dhparam"].as<std::string>());
    server.setPrivateKeyFile(vm["pkey"].as<std::string>());
    server.run();

  } catch (exception &e) {
    std::cout << e.what() << "\n";
    return 1;
  }
}
