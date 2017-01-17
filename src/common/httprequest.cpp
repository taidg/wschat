#include "httprequest.h"

#include <curl/curl.h>
#include <boost/bind.hpp>
#include <cstring>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

HttpRequest::HttpRequest(std::string url) {
  CURL *easyHandle = curl_easy_init();

  curl_easy_setopt(easyHandle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, (void *) this);
  curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, write_callback);
  errorCode_ = curl_easy_perform(easyHandle);
  curl_easy_cleanup(easyHandle);
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  HttpRequest *request = (HttpRequest *) userdata;

  request->buffer_.append(ptr, size * nmemb);

  return size * nmemb;
}

bool HttpRequest::good() {
  return !errorCode_;
}

std::string HttpRequest::data() {
  return buffer_;
}

std::string HttpRequest::escape(std::string str) {
  CURL *curl = curl_easy_init();
  char *output = curl_easy_escape(curl, str.data(), str.length());
  std::string result = output;
  curl_free(output);
  curl_easy_cleanup(curl);
  return result;
}
