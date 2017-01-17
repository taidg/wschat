#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>

class HttpRequest {
 public:
  HttpRequest(std::string url);
  bool good();
  std::string data();
  static std::string escape(std::string str);

 private:
  friend size_t write_callback(char *ptr, size_t size, size_t nmemb,
                               void *userdata);
  std::string buffer_;
  int errorCode_;
};

#endif // HTTPREQUEST_H
