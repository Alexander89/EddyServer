#ifndef GAMESESSION_H
#define GAMESESSION_H
#include "webSession.h"

enum Status : short {
  Waiting,
  StartUp,
  InGame,
  Finish,
};

class GameSession : public WebSession
{
public:
  explicit GameSession( int socket );
  explicit GameSession( std::wstring username );
  explicit GameSession( );
  ~GameSession();

  virtual void handleContinuationData( char* data, int lng );
  virtual void handleTextData( char* data, int lng );
  virtual void handleBinaryData( char* data, int lng );
  virtual void clientDisconnected();

  std::wstring getPlayerJSON();
  void resetPlayer();

public:
  std::wstring m_username;
  uint m_userID{0};
  int m_reputation{0};
  uint m_exp{0};

  bool m_ready{false};
  bool m_prefSeeker{false};
  bool m_loadGameDone{false};
  bool m_inGame{false};

  std::wstring m_skillData;

  float m_x{0.f};
  float m_y{0.f};
  float m_z{0.f};
  float m_rotZ{0.f};
  int m_health{100};
  bool m_catched{false};
  bool m_died{false};
  bool m_hidden{false};
  float m_catchTime{0.f};
  float m_diedTime{0.f};
};
#endif