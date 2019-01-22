#include "webSession.h"
#include "helperfunctions.h"
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <clocale> 
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

using namespace std;

constexpr int c_bufferSize = 10240;


WebSession::WebSession( int socket ) :
  m_socket(socket) 
{
}
WebSession::~WebSession() {

}

void* runWebSessionThread(void* self) {
  std::setlocale(LC_ALL, "de_DE.utf8");
  static_cast<WebSession*>(self)->run();
  return nullptr;
} 
void WebSession::start() {
  pthread_create( &m_threadID, nullptr, runWebSessionThread, (void*)this );  
}
void WebSession::run() {
  char dataBuffer[c_bufferSize];
  cout << "start new WebSession" << endl;
  int size = readHTMLData(dataBuffer,c_bufferSize);
  if ( size < 0 ){
    cout << "kann html-Paket nicht lesen " << size << endl;  
    close( m_socket );//Close the socket  
    return;
  }
  handleGetRequest( dataBuffer, size );

  while ( true ) {
    if ( m_terminate ) {
      clientDisconnected();
      close( m_socket );
      break; 
    }
    timeval networkTimeout{ .tv_sec = 10, .tv_usec = 0 };
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_socket, &readfds);
    select(m_socket+1, &readfds, NULL, NULL, &networkTimeout);
    if ( !FD_ISSET(m_socket, &readfds) ) {
      if ( m_pingSend >= 2 ) {
        std::cout << "pong missing: disconnected close" << std::endl;
        clientDisconnected();
        close(m_socket);
        break;
      }
      sendPing();
      continue;
    } 
    else {
      int size = this->readData(dataBuffer, c_bufferSize);
      if (size <= 0) { //Somebody disconnected , get his details and print
        int addrlen;
        sockaddr_in address;
        getpeername(m_socket , (struct sockaddr*)&address , (socklen_t*)&addrlen);
        cout << "Host disconnected , ip " << inet_ntoa(address.sin_addr) <<" , port "<< ntohs(address.sin_port) << endl;
        clientDisconnected();
        close( m_socket );//Close the socket
        break;
      }      
      else //handle SocketData
        handleCompleatingData(dataBuffer, size);
    }
  }
}

void WebSession::handleGetRequest( const char* data, int lng ) {
  string request;
  request.assign(data, lng);
  auto v = splitString(request, "\r\n");

  for (auto i: *v) {
    auto line = splitString( i, ":" );
    if ( line->size() && line->at(0) == "Sec-WebSocket-Key" ) {
      string key = line->at(1).substr(1);
      const char *acceptkey = createAcceptKey( key.c_str(), key.size() );
      m_returnkey = string(acceptkey);
    }
    if ( line )
      delete line;
    
  }

  string reply("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Protocol: soap\r\nSec-WebSocket-Accept:");
  reply.append(m_returnkey);
  reply.append("\r\n\r\n");

  send( m_socket, reply.c_str(), reply.size(), 0 );
  request.clear();
  reply.clear();
  delete v;
}

void WebSession::handleCompleatingData( char* data, int lng ) {
  WebSocketDataFrame *frame = reinterpret_cast<WebSocketDataFrame*>(data);

  if ( lng < frame->minLng() ) {
    int missing = frame->minLng() - lng;
    lng += readData(data + lng, c_bufferSize-lng, missing); // daten hinten an den Puffer anfügen
  }

  int frameLng = frame->framesize();
  char* newbuffer{nullptr};
  if ( frameLng > c_bufferSize ) {
    cout << "read more to larger Buffer" << endl;
    newbuffer = new char[frameLng];
    memcpy( newbuffer, data, lng );
    lng += readData(newbuffer + lng, frameLng-lng, frameLng-lng);
    frame = reinterpret_cast<WebSocketDataFrame*>(newbuffer);
  }
  else if ( lng < frameLng ) {
    lng += readData(data + lng, c_bufferSize-lng, frameLng-lng);
  }

  WebSocketData request(frame);

  if ( frame->opcode() > 7 ) // control frame
    handleObcodeFream(request);  
  else // data frame
    collectDataFrames(request);
  
  if ( newbuffer ){
    delete[] newbuffer; // delete nullptr is ok
  }
}

void WebSession::handleObcodeFream( WebSocketData &request ) {
  switch ( request.opcode() ){
    case OPCODE::oc_close: 
      close(m_socket);
      m_terminate = true;;
    break;
    case OPCODE::oc_ping:{
      WebSocketData reply( oc_pong, nullptr, 0 );
      reply.sendPackage(m_socket);
    }
    break;
    case OPCODE::oc_pong: // reset keapalive timer
      m_pingSend = 0;
    break;
    default: break;
  } 
}

int WebSession::sendReply( OPCODE op, const wchar_t* data, int lng ) {
  WebSocketData reply( op, data, lng );
  return reply.sendPackage( m_socket );
}
void WebSession::sendPing(){
  m_pingSend++;
  WebSocketData reply( OPCODE::oc_ping, L"hallo", 10 );
  reply.sendPackage( m_socket );
  return;
}
void WebSession::collectDataFrames ( WebSocketData &request ) {
  m_pingSend = 0; // reset keapalive timer, opponent is available

  // Daten einheitlich über das Request Sammelsystem vorbereiten 
  if ( m_currentRequestLng == 0 ){ // new request
    m_currentRequestData = new char[ request.dataLng() ];
  } 
  else {
    const char *temp = m_currentRequestData;
    m_currentRequestData = new char[ m_currentRequestLng + request.dataLng() ];
    memcpy( m_currentRequestData, temp, m_currentRequestLng); 
    delete[] temp;
  }
  memcpy( m_currentRequestData + m_currentRequestLng, request.data(), request.dataLng()); 
  m_currentRequestLng += request.dataLng();

  // Fertige Daten abarbeiten, sonst auf wetiere Daten warten und weiter sammeln
  if ( request.fin() ) {
    switch ( request.opcode() ) {
      case OPCODE::oc_continuation: handleContinuationData( m_currentRequestData, m_currentRequestLng ); break;
      case OPCODE::oc_text: handleTextData( m_currentRequestData, m_currentRequestLng ); break;
      case OPCODE::oc_binary: handleBinaryData( m_currentRequestData, m_currentRequestLng ); break;
      default: break;
    }    
    // Sind die Daten komplett und abgearbeitet, so können Sie gelöscht werden.
    delete[] m_currentRequestData;
    m_currentRequestData = nullptr;
    m_currentRequestLng = 0;
  }
}

int WebSession::readHTMLData( char *dataBuffer, int maxSize ) {
  int bufferWalker{0};
  bool isHTML{false};
  do {
    int dataRec = recv(m_socket, dataBuffer + bufferWalker, maxSize-bufferWalker, 0);
    if (dataRec < 0) // -1 == Fehler
      return -1; // restliche verarbeitung unterbinden

    bufferWalker += dataRec;

    if ( !isHTML && bufferWalker > 4 ) {
      if ( memcmp(dataBuffer, "GET", 3) )
        return -2; // kein GET request erhalten, 
      isHTML = true;
    }

  } while ( memcmp(dataBuffer+bufferWalker-4, "\r\n\r\n", 4) );
  dataBuffer[bufferWalker] = '\0';
  return bufferWalker;
}

int WebSession::readData( char *dataBuffer, int maxSize, int minSize ) {
  int bufferWalker{0};
  int dataRec{0};
  do {
    int dataRec = recv(m_socket, dataBuffer + bufferWalker, maxSize-bufferWalker, 0);
    if (dataRec < 0)
      return -1; // restliche verarbeitung unterbinden
    bufferWalker += dataRec;
    usleep(5);
  } while ( dataRec > 0 || bufferWalker < minSize );
  dataBuffer[bufferWalker] = '\0';
  return bufferWalker;
}

