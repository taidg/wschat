#ifndef ASIOSERVER_H
#define ASIOSERVER_H

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

class custom_data;

struct connection_data {
  custom_data *data;
};

struct custom_config : public websocketpp::config::asio_tls {
  // pull default settings from our core config
  typedef websocketpp::config::asio_tls core;

  typedef core::concurrency_type concurrency_type;
  typedef core::request_type request_type;
  typedef core::response_type response_type;
  typedef core::message_type message_type;
  typedef core::con_msg_manager_type con_msg_manager_type;
  typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;
  typedef core::alog_type alog_type;
  typedef core::elog_type elog_type;
  typedef core::rng_type rng_type;
  typedef core::transport_type transport_type;
  typedef core::endpoint_base endpoint_base;

  // Set a custom connection_base class
  typedef connection_data connection_base;
};


typedef websocketpp::server<custom_config> AsioServer;

typedef AsioServer::connection_ptr connection_ptr;
typedef websocketpp::config::asio_tls::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

// See https://wiki.mozilla.org/Security/Server_Side_TLS for more details about
// the TLS modes. The code below demonstrates how to implement both the modern
enum tls_mode {
  MOZILLA_INTERMEDIATE = 1,
  MOZILLA_MODERN = 2
};

context_ptr on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl,
                        std::string dh_filename, std::string private_key_filename);


#endif // ASIOSERVER_H

