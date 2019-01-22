#ifndef WebSession_H
#define WebSession_H

#include <string>
#include "webSocketFrame.h"
#include <pthread.h>
struct WebSocketDataFrame;

enum RequestType {
  CHECKUSERNAME,
  GETSERVERLIST,
  SUBSCRIBEROOMDATA,
  UNSUBSCRIBEROOMDATA,
};


class WebSocketData;

class WebSession
{
public:
  explicit WebSession( int socket );
  ~WebSession();

  void start();
  void run();
  uint pid() const { return (uint)m_threadID; }


  void handleGetRequest( const char* data, int lng );
  void handleCompleatingData ( char* data, int lng );
  void collectDataFrames ( WebSocketData &request  );
  void handleObcodeFream( WebSocketData &request );
  virtual void handleContinuationData( char* data, int lng ) = 0;
  virtual void handleTextData( char* data, int lng ) = 0;
  virtual void handleBinaryData( char* data, int lng ) = 0;
  virtual void clientDisconnected( ) = 0;

  int sendReply( OPCODE op, const wchar_t* data, int lng );
  void sendPing();

private:
  int readHTMLData( char *dataBuffer, int maxSize );
  int readData( char *dataBuffer, int maxSize, int minSize = 4 );

private:
  pthread_t m_threadID{0};
  int m_socket{0};
  bool m_terminate{false};
  char* m_currentRequestData{nullptr};
  unsigned int m_currentRequestLng{0};

  uint m_pingSend{0};
  std::string m_returnkey;  
};


#endif