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
  HTTPRequestHandler(){
    blacklist.addToBlacklist(blacklistFile);
  };

  void serviceRequest(const std::pair<int, std::string>& connection) throw();
  void clearCache();
  void setCacheMaxAge(long maxAge);
 private:
  const std::string blacklistFile = "blocked-domains.txt";
  HTTPBlacklist blacklist;
  HTTPCache cache;
};

#endif
