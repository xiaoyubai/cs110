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

void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection) throw() {
  // parse request from client
  sockbuf csb(connection.first);
  iosockstream css(&csb);
  HTTPRequest request;
  request.ingestRequestLine(css);
  request.ingestHeader(css, connection.second);
  request.ingestPayload(css);

  // modify request by client
  HTTPHeader& header = request.getHeader();
  header.addHeader("x-forwarded-proto", "http");
  string forwardedForStr;
  if (header.containsName("x-forwarded-for"))
    forwardedForStr += header.getValueAsString("x-forwarded-for") + ",";
  forwardedForStr += connection.second;
  header.addHeader("x-forwarded-for", forwardedForStr);

  // send modified request to host
  int hs = createClientSocket(request.getServer(), request.getPort());
  sockbuf hsb(hs);
  iosockstream hss(&hsb);
  hss << request << flush;
//  cout << request.getURL() << endl;

  // receive response from host
  HTTPResponse response;
  response.ingestResponseHeader(hss);
  response.ingestPayload(hss);
//  cout << response.getResponseCode() << endl;

  // write response to client
  css << response << flush;
}

// the following two methods needs to be completed 
// once you incorporate your HTTPCache into your HTTPRequestHandler
void HTTPRequestHandler::clearCache() {}
void HTTPRequestHandler::setCacheMaxAge(long maxAge) {}
