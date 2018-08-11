/**
 * File: request-handler.h
 * -----------------------
 * Defines the HTTPRequestHandler class, which fully proxies and
 * services a single client request.  
 */

#ifndef _request_handler_
#define _request_handler_

#include <utility>
#include <string>
#include "cache.h"
#include "blacklist.h"

class HTTPRequestHandler {
 public:
  HTTPRequestHandler(){ blacklist.addToBlacklist(blacklistFile); };
  size_t getMutexHash(const HTTPRequest& request);
  void serviceRequest(const std::pair<int, std::string>& connection) throw();
  void clearCache();
  void setCacheMaxAge(long maxAge);
  void setProxy(const std::string server, unsigned short port);

 private:
  const std::string blacklistFile = "blocked-domains.txt";
  HTTPBlacklist blacklist;
  HTTPCache cache;
  bool isUsingProxy = false;
  std::string proxyServer;
  unsigned short proxyPort;
};

#endif
