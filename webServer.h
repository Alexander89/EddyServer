#include "server.h"
#include <string>
#include <vector>


class WebServer : public Server
{
public:
  WebServer( unsigned short port );
  ~WebServer();
  
  void handleHttp( const char* data, int lng );
  void handleData( const char* data, int lng );
  virtual void receveData( const char *data, int lng);

  virtual void clientConnected( std::string request ) = 0;
  virtual void handleRequest( const char *data, int lng ) = 0;


private:
  std::vector<std::string> splitString( std::string req, const char *seperator, std::vector<std::string> vec = std::vector<std::string>() );
};