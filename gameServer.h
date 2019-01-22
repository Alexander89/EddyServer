#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <functional>
#include <thread>
#include <pthread.h>
#include "gameSession.h"
#include "server.h"

class Room;
struct timerEntry {
  uint                  m_id;
  uint                  m_counter{1};
  uint                  m_interval; // ms
  bool                  m_singleShot{false};
  std::function<void(uint)> m_callback;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};

class GameServer
{
public:
  GameServer();
  ~GameServer();

  void start();
  void stop();

  void timerThread();
  uint setTimer( uint timeout, std::function<void(uint)> callback, bool singleshot = false );
  void stopTimer( uint timerID );

  void clientDisconnected( GameSession *user );

  bool joinRoom( GameSession *user, int roomId );
  void leaveAllRooms( GameSession *user );

  void addChatMessage( GameSession *user, std::wstring msg );
  void mapSuggestion( GameSession *user, uint mapID );
  void voteMap( GameSession *user, bool up );
  void publishPlayerData( GameSession *user, std::wstring msg );
  void leftResult( GameSession *user );
  void setReady( GameSession *user );
  void setReadySeeker( GameSession *user );
  void setLoadGameReady( GameSession *user );

  std::vector< std::vector<std::wstring> > getRoomList();
  std::wstring getRoomJSONList();
  std::wstring getRoomJSON(uint id);
  
private:
  Room *getRoom(uint id);
  Room *createNewRoom();
  bool isEmptyRoomAvailable();
  void keepFirstEmptyRoom();

private:
  bool m_terminate{false};
  Server m_server;
  std::vector<Room *> m_rooms;
  std::map<int, Room*> m_assignedSessions;

  pthread_t m_timerThreadID{0};
  std::map<uint, timerEntry*> m_timers;
  uint m_lastTimerID{0};
};

extern GameServer *g_gameSvr;
#endif // GAMESERVER_H