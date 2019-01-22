#ifndef SERVER_H
#define SERVER_H
#include <vector>
#include <pthread.h>

class GameSession;

class Server {
public:
  explicit Server( unsigned short port );

  void start();
  void run();
  void stop() { m_terminate = true; }
  bool isRunning() { return m_socket != 0; }


private:
  pthread_t m_threadID{0};
  int m_socket{0};
  bool m_terminate{false}; 

  std::vector<GameSession *> m_sessions;
};

#endif