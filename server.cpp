#include "gameSession.h"
#include "server.h"

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

Server::Server( unsigned short port ) {
  m_socket = socket( AF_INET, SOCK_STREAM, 0);
  if (m_socket == -1) {
    std::cout << "Socket kann nicht erstellt werden" << std::endl;
  }

  sockaddr_in addres{0};
  addres.sin_family = AF_INET;
  addres.sin_port = htons( port );
  addres.sin_addr.s_addr = INADDR_ANY;
  const int y = 1;
  setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int) );
  if ( bind(m_socket, (sockaddr*) &addres, sizeof(addres) ) ) {
    std::cout << "Socket kann nicht auf port " << port << " gebunden werden."<< std::endl;
    exit(1);
  }
}

void* runServerThread(void* self) {
  static_cast<Server*>(self)->run();
  return nullptr;
} 
void Server::start() {
  listen( m_socket, 100 );
  pthread_create( &m_threadID, nullptr, runServerThread, this );  
}
void Server::run() {
  sockaddr_in clientInfo{0};
  socklen_t dataLng = sizeof(clientInfo);
  while( !m_terminate ) {
    // accept besser durch pool ersetzen, da sonst der socket nicht richtig geschlossen werden kann.
    int client = accept( m_socket, reinterpret_cast<sockaddr*>(&clientInfo), &dataLng );
    std::cout << "incomming connection "<< std::endl;
    m_sessions.push_back( new GameSession(client) );
  }
  close(m_socket);
  m_socket = 0;
}





