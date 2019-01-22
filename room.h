#ifndef ROOM_H
#define ROOM_H
#include <vector>
#include <string>
#include <chrono>
#include "lvl.h"
#include "gameSession.h"

//
class selectLvl;
class GameSession;

//enum Status : short {}// in GameSession verschoben

enum class PlayerType {
  Undefined,
  Seeker,
  Player,
};
enum class GameEnd {
  Catched,
  Died,
  Left,
  TimeOver,
};

struct LeftPlayers {
  LeftPlayers(){};
  LeftPlayers(GameSession* s);
  LeftPlayers *setLeftTime( float sec ) { m_leftAfterSeconds = sec; return this; }
  std::wstring m_username;
  uint m_userID{0};
  float m_leftAfterSeconds{0};

  bool m_wasSeeker{false};
  bool m_wasCatched{false};
  bool m_wasDath{false};
  uint m_DiedOrCatchedAfterSeconds{0};
};

class Room
{
public:
  Room();
  Room( Room &o );
  ~Room();

  std::wstring getJsonOverview();
  std::wstring getJson();
  std::wstring playersJson();
  std::wstring playerStatsJson();

  Room* selectLvl( uint lvlID );
  bool isEmpty() const { return m_clients.size() == 0; } 

  bool joinRoom( GameSession* s );
  Room* leaveRoom( GameSession* s );
  void leftResult( GameSession *user );
  void updateStat( GameSession *user );
  void addChatMsg( GameSession *user, std::wstring msg );
  void mapSuggestion( GameSession *user, uint mapID ); 
  void registerVote( bool voteUp );
  void voteMapFinal(); 
  void publishPlayerData( GameSession *user, std::wstring msg );
  void publishNewMap( );
  void playerReady();

  Status status() const { return m_status; }
  uint id() const { return m_id; }
  std::wstring name() const { return m_name; }
  int connections() const { return m_clients.size(); }

private:
  void updateGamePlay();
  void publishResult();
  void startGameTimer();
  void startGame();
  void setupGameTimeout();
  void stopGameTimer();
  void setGameEnd( PlayerType winner, GameEnd reason );
  void fixSeeker();
  void informRoomAboutNewClient( GameSession *newClient );
  void informRoomAboutNewStat();

  void sendWebGlInitPackage();

  void sendToAll( std::wstring msg, GameSession *exept=nullptr );

private:
  static uint m_maxID;
  uint m_id;
  bool m_openMapVote{false};
  int m_mapVote{0};
  int m_mapVotesMissing{0};
  uint m_voteForMapID{0};
  std::wstring m_name;
  std::vector< GameSession* > m_clients;
  std::vector< LeftPlayers* > m_leftPlayers;
  Status m_status {Status::Waiting};
  Lvl* m_lvl{nullptr};

  std::chrono::high_resolution_clock::time_point m_startTime;
  uint m_startGame{0};
  uint m_startGameTimer{0};

  uint m_gameDuration{0};
  uint m_gameTimer{0};

  PlayerType  m_winner{ PlayerType::Undefined };
  GameEnd     m_gameEndReason{ GameEnd::TimeOver };
  float       m_gameTime{ 0.0f };
};
#endif