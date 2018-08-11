/**
 * File: request-handler.cc
 * ------------------------
 * Provides the implementation for the HTTPRequestHandler class.
 */

#include "client-socket.h"
#include "request.h"
#include "request-handler.h"
#include "response.h"
#include <sstream>
#include <socket++/sockstream.h> // for sockbuf, iosockstream
#include "ostreamlock.h"
using namespace std;

size_t HTTPRequestHandler::getMutexHash(const HTTPRequest& request) {
  return cache.hashRequest(request) % cache.numMutex;
}

void HTTPRequestHandler::setProxy(const std::string server, unsigned short port) {
  isUsingProxy = true;
  proxyServer = server;
  proxyPort = port;
}

void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection) throw() {
  // parse request from client
  sockbuf csb(connection.first);
  iosockstream css(&csb);
  HTTPRequest request;
  request.ingestRequestLine(css);
  request.ingestHeader(css, connection.second);
  request.fixRequestServerField();
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

  // check for proxy cycle
  HTTPHeader& header = request.getHeader();
  if (isUsingProxy && header.containsName("x-forwarded-for")) {
    string token;
    istringstream ss(header.getValueAsString("x-forwarded-for"));
    while (getline(ss, token, ',')) {
      if (token == proxyServer) {
        response.setResponseCode(504);
        response.setProtocol("HTTP/1.0");
        response.setPayload("Proxy chain cycle found");
        css << response << flush;
        return;
      }
    }
  }

  // modify request by client
  bool isFromProxy = header.containsName("x-forwarded-for");
  cout << ((isFromProxy) ? "IS FROM PROXY!!" : "not proxy") << endl;
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
  string originHostServer = request.getServer();
  if (isFromProxy) {
    if (!header.containsName("host")) {
      response.setResponseCode(400);
      response.setProtocol("HTTP/1.0");
      response.setPayload("Bad Request");
      css << response << flush;
      return;
    }
//    originHostServer = header.getValueAsString("host");
  }

//  if (isUsingProxy) {
//    request.
//  }

  const std::string& hostServer = (isUsingProxy) ? proxyServer : originHostServer;
  unsigned short hostPort = (isUsingProxy) ? proxyPort : request.getPort();
  cout << "hostServer: " << hostServer << ", hostPort: " << hostPort << endl;
  int hs = createClientSocket(hostServer, hostPort);
  if (hs == kClientSocketError) {
//    cout << "can't create socket to origin" << endl;
    response.setResponseCode(400);
    response.setProtocol("HTTP/1.0");
    response.setPayload("Bad Request");
    css << response << flush;
    return;
  }
  sockbuf hsb(hs);
  iosockstream hss(&hsb);
  hss << request << flush;

  // receive response from host
  response.ingestResponseHeader(hss);
  response.ingestPayload(hss);

  // write response to client
  cout << oslock << request << endl <<  response.getResponseCode() << osunlock<< endl;
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
