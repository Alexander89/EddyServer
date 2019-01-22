#include <iostream>
#include <gameServer.h>
#include <unistd.h>
#include <string.h>

using namespace std;
int main ()
{
  std::cout << "new GameServer " << std::endl;
  GameServer* s = new GameServer();
  s->start();

  bool running = true;
  while (running) {
    usleep(100);
    string line;
    getline(cin, line);
    if ( line.size() ) {
      cout << line << endl;
      if ( line == "exit" || line == "quit") {
        s->stop();
      running = false;
      }     
    }   
  }
  return 0;
}
