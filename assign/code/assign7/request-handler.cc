/**
 * File: request-handler.cc
 * ------------------------
 * Provides the implementation for the HTTPRequestHandler class.
 */

#include "client-socket.h"
#include "request.h"
#include "request-handler.h"
#include "response.h"
#include <socket++/sockstream.h> // for sockbuf, iosockstream
using namespace std;

size_t HTTPRequestHandler::getMutexHash(const HTTPRequest& request) {
  return cache.hashRequest(request) % cache.numMutex;
}

void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection) throw() {
  // parse request from client
  sockbuf csb(connection.first);
  iosockstream css(&csb);
  HTTPRequest request;
  request.ingestRequestLine(css);
  request.ingestHeader(css, connection.second);
  request.ingestPayload(css);

  // check if blacklisted
  HTTPResponse response;
  if (!blacklist.serverIsAllowed(request.getServer())) {
    response.setResponseCode(403);
    response.setProtocol("HTTP/1.0");
    response.setPayload("Forbidden Content");
    css << response << flush;
    return;
  }

  // modify request by client
  HTTPHeader& header = request.getHeader();
  header.addHeader("x-forwarded-proto", "http");
  string forwardedForStr;
  if (header.containsName("x-forwarded-for"))
    forwardedForStr += header.getValueAsString("x-forwarded-for") + ",";
  forwardedForStr += connection.second;
  header.addHeader("x-forwarded-for", forwardedForStr);

  // attempt to acquire mutex for this request
  size_t hash = getMutexHash(request);
  lock_guard<mutex> lg(cache.ms[hash]);

  // check if cached only after modifying
  // (because i will cache the modified request)
  if (cache.containsCacheEntry(request, response)) {
    css << response << flush;
    return;
  }

  // send modified request to host
  int hs = createClientSocket(request.getServer(), request.getPort());
  sockbuf hsb(hs);
  iosockstream hss(&hsb);
  hss << request << flush;

  // receive response from host
  response.ingestResponseHeader(hss);
  response.ingestPayload(hss);

  // write response to client
  css << response << flush;

  // cache if necessary
  if (cache.shouldCache(request, response)) cache.cacheEntry(request, response);
}

// the following two methods needs to be completed 
// once you incorporate your HTTPCache into your HTTPRequestHandler
void HTTPRequestHandler::clearCache() {
  cache.clear();
}

void HTTPRequestHandler::setCacheMaxAge(long maxAge) {
  cache.setMaxAge(maxAge);
}
