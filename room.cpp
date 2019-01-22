#include "room.h"
#include "lvlMgr.h"
#include "userMgr.h"
#include "gameSession.h"
#include "gameServer.h"
#include "helperfunctions.h"
#include <sstream>
#include <cstdlib>
#include <time.h> 
#include <math.h>  

const std::vector<std::wstring> nameList = {L"Alice",L"Lilly",L"Maja",L"Elsa",L"Ella",L"Alicia",L"Olivia",L"Julia",L"Ebba",L"Wilma",L"Saga",L"Agnes",L"Freja",L"Alma",L"Astrid",L"Ellie",L"Molly",L"Alva",L"Ellen",L"Stella",L"Clara",L"Linnea",L"Emma",L"Signe",L"Isabelle",L"Vera",L"Leah",L"Emilia",L"Ester",L"Sara",L"Nova",L"Ines",L"Nellie",L"Selma",L"Sigrid",L"Elise",L"Sofia",L"Juni",L"Elvira",L"Elin",L"Isabella",L"Lovisa",L"Iris",L"Liv",L"Tyra",L"Celine",L"Edith",L"Felicia",L"Meja",L"Moa",L"Livia",L"Nora",L"Tuva",L"Ida",L"Thea",L"Siri",L"Leia",L"Maria",L"Matilda",L"Hanna",L"Majken",L"Lykke",L"Tilde",L"Stina",L"Lova",L"Amanda",L"Melissa",L"Ingrid",L"Luna",L"Sally",L"My",L"Jasmine",L"Ronja",L"Rut",L"Joline",L"Svea",L"Märta",L"Linn",L"Cornelia",L"Amelia",L"Emelie",L"Hilda",L"Cleo",L"Adele",L"Filippa",L"Hilma",L"Penny",L"Lovis",L"Tilda",L"Ellinor",L"Julie",L"Lo",L"Elina",L"Emmy",L"Greta",L"Bianca",L"Mila",L"Nathalie",L"Novalie",L"Hedda",L"Oscar",L"Lucas",L"William",L"Liam",L"Oliver",L"Hugo",L"Alexander",L"Elias",L"Charlie",L"Noah",L"Adam",L"Ludvig",L"Filip",L"Adrian",L"Axel",L"Nils",L"Alfred",L"Leo",L"Vincent",L"Elliot",L"Leon",L"Arvid",L"Harry",L"Valter",L"Isak",L"Theodor",L"Viktor",L"Melvin",L"Edvin",L"Benjamin",L"Theo",L"Gustav",L"Emil",L"Olle",L"Mohamed",L"Viggo",L"Erik",L"Albin",L"Sixten",L"Loui",L"Love",L"Gabriel",L"Ebbe",L"Melker",L"August",L"Vidar",L"Malte",L"Josef",L"Frank",L"Max",L"Casper",L"Anton",L"Henry",L"Noel",L"Wilmer",L"Jacob",L"Sam",L"Jack",L"Elton",L"Carl",L"Sigge",L"Loke",L"Kevin",L"Matteo",L"Tage",L"Felix",L"Colin",L"Milo",L"Alvin",L"Milton",L"Ville",L"Hjalmar",L"Wilhelm",L"Otto",L"Frans",L"Jonathan",L"Aron",L"Kian",L"Nicolas",L"Julian",L"Simon",L"Vilgot",L"David",L"Elis",L"Ivar",L"Elvin",L"Joel",L"John",L"Ali",L"Samuel",L"Daniel",L"Thor",L"Maximilian",L"Rasmus",L"Sebastian",L"Folke",L"Omar",L"Milian",L"Mio",L"Edward"};
const std::wstring statusText[4]{L"Waiting",L"Startet",L"In Game",L"Finish"};


LeftPlayers::LeftPlayers( GameSession* s ){
  m_username = s->m_username;
  m_userID = s->m_userID;
  m_wasSeeker = s->m_prefSeeker;
  m_wasCatched = s->m_catched;
  m_wasDath = s->m_died;
  if ( s->m_catched )
    m_DiedOrCatchedAfterSeconds = s->m_catchTime;
  else if ( s->m_died )
    m_DiedOrCatchedAfterSeconds = s->m_diedTime;
}

uint Room::m_maxID{0};
Room::Room() :
  m_id( m_maxID++ ),
  m_name( nameList[rand() % 200] ),
  m_lvl( g_lvlMgr->getLvl(0) )
{
}
Room::Room( Room &o ) 
{
  std::cout << "mk copy " << std::endl;
  m_id = o.m_id;
  m_name = o.m_name;
  m_clients = o.m_clients;
  m_status = o.m_status;
  m_lvl = o.m_lvl;
}
Room::~Room() {
}


std::wstring Room::getJsonOverview() {
  std::wstring res;
  res += L"{";
  res += L"\"id\":" + std::to_wstring(m_id) + L",";
  res += L"\"name\":\"" + m_name + L"\",";
  res += L"\"con\":\"" + std::to_wstring( connections() )  + L"\",";
  res += L"\"state\":\"" + statusText[(int)m_status]  + L"\",";
  if ( m_lvl )  // sollte immer wahr sein
    res += L"\"room\":" + m_lvl->asJsonNoData();
  res += L"}";
  return res;
}
std::wstring Room::getJson() {
  std::wstring res;
  res += L"{";
  res += L"\"id\":" + std::to_wstring(m_id) + L",";
  res += L"\"name\":\"" + m_name + L"\",";
  res += L"\"con\":\"" + std::to_wstring( connections() )  + L"\",";
  res += L"\"state\":\"" + statusText[(int)m_status]  + L"\",";
  if ( m_lvl )  // sollte immer wahr sein
    res += L"\"map\":" + m_lvl->asJson() + L",";
  res += L"\"players\":" + playersJson();
  res += L"}";
  return res;
}
std::wstring Room::playersJson() {
  std::wstring res;
  res += L"[";
  bool first{true};
  for ( GameSession* s : m_clients ) {
    if ( !first )
      res += L",";
    first = false;
    res += s->getPlayerJSON();
  }
  res += L"]";
  return res;
}
std::wstring Room::playerStatsJson() {
  std::wstring res;
  res += L"[";
  bool first{true};
  uint nr{0};
  for ( GameSession* s : m_clients ) {
    if ( !first )
      res += L",";
    first = false;
    res += L"{\"nr\":" + std::to_wstring(nr) + L", \"stat\":\"";
    if ( s->m_ready && s->m_prefSeeker )
      res += L"Seek";
    else if ( s->m_ready )
      res += L"Ready";
    else
      res += L"Open";
    res += L"\"}";
    nr++;  
  }
  res += L"]";
  return res;
}

Room* Room::selectLvl( uint lvlID ) {
  Lvl *l = g_lvlMgr->getLvl( lvlID );
  if ( l->valid() )
    m_lvl = l;
  return this;
}

bool Room::joinRoom(GameSession* s) {
  // already connected? return true so Player is in the room (snh)
  for ( auto itr = m_clients.begin(); itr != m_clients.end(); itr++ )
    if ( *itr == s )
      return true;

  // add Player if there is a free slot.
  if ( m_lvl && m_clients.size() < m_lvl->maxPlayer() ) {
    m_clients.push_back(s);
    s->resetPlayer();
    informRoomAboutNewClient( s );
    return true;
  }
  return false; 
}
Room* Room::leaveRoom( GameSession* s ) {
  int nr{-1};
  for ( auto itr = m_clients.begin(); itr != m_clients.end(); itr++ ){
    nr++; 
    if ( *itr == s ){
      m_clients.erase(itr);
      informRoomAboutNewClient( s );
      
      if ( m_status == Status::InGame || m_status == Status::StartUp ) {
        g_userMgr->changeRepu( s, -10 );
        using namespace std::chrono;
        m_leftPlayers.push_back( (new LeftPlayers(s))->setLeftTime(duration_cast<seconds>(high_resolution_clock::now() - m_startTime).count() - 3) );
      }
      break;
    }
  }
  if ( m_openMapVote ) 
    registerVote(true);

  if ( m_status == Status::Waiting ) {
    // do logic in menue
    updateStat( s );
  }
  else if ( m_status == Status::InGame || m_status == Status::StartUp ) {
    // do game logic
    s->m_inGame = false;
    std::wcout << nr << L" " << s->m_username << L"left the room. Seeker:" << s->m_prefSeeker << std::endl;
    if ( s->m_prefSeeker ) {
      sendToAll( L"GP_upd:{ \"type\":\"PlayerWins\", \"reason\":2 }" );
      setGameEnd( PlayerType::Player, GameEnd::Left );
    }
    else {
      sendToAll( std::wstring(L"GP_upd:{ \"type\":\"Left\", \"nr\":") + std::to_wstring( nr ) + L", \"name\":\"" + s->m_username + L"\" }");
      if ( m_clients.size() == 1 ) {
        std::wcout << L"Just one left" << std::endl;
        sendToAll( L"GP_upd:{ \"type\":\"SeekerWins\", \"reason\":2 }" );
        setGameEnd( PlayerType::Seeker, GameEnd::Left );
      }
    }
    updateGamePlay();
  }

  if ( m_clients.size() == 0 ){
    stopGameTimer();
    m_status = Status::Waiting;
  }
  return this;
}

void Room::leftResult( GameSession *user ) {
  uint inGame{0};
  for ( auto c : m_clients ) 
    inGame += c->m_inGame ? 1 : 0;
  
  if ( inGame == 0 )
    m_status = Status::Waiting;
}
void Room::updateStat( GameSession *user ){
  // teste ob min 2 Spieler da sind und alle den Status ready haben,
  // dann counter für spiel start anfangen
  if ( m_clients.size() >= 2 ) {
    bool allReady{true};
    for ( auto c : m_clients )
      if ( !c->m_ready )
        allReady = false;
    
    if ( allReady )
      startGameTimer();
  }
  informRoomAboutNewStat( );
}
void Room::addChatMsg( GameSession *user, std::wstring msg ) {
  int nr{1};  // hat nen zusatzefekt für user die nicht im zimmer angemeldet sind (sys MSG)
  for ( auto c : m_clients ) {
    if ( user == c )
      break;
    nr++;
  }
  std::wstring res(L"Chat:{\"user\":\"");
  res += user->m_username + L"\", \"msg\":\"";
  res += msg + L"\",\"id\":" + std::to_wstring(nr) + L"}";

  sendToAll( res );
}
void Room::mapSuggestion( GameSession *user, uint mapID ) {
  if ( m_openMapVote )
    return;
  m_openMapVote = true;

  Lvl *l = g_lvlMgr->getLvl(mapID);
  if ( !l->valid() ){
    m_openMapVote = false;
    return;
  }

  m_voteForMapID = mapID;
  m_mapVote = 0;
  m_mapVotesMissing = m_clients.size();


  uint nr{1};
  for( auto c: m_clients ) {
    std::wstring res;
    std::wostringstream ss(res);
    std::cout << " make info for Player "<< std::endl;
    ss << L"MapVote:{\"name\":\"" << user->m_username << L"\",";
    if ( nr > l->maxPlayer() )
      ss << L" \"impNote\":\"Achtung: Für dich wäre kein Platz mehr auf der Karte frei!\",";
    ss << L"\"map\":" << l->asJson() << L"}";
    res = ss.str();
    std::cout << " send info an Player "<< std::endl;
    c->sendReply( oc_text, res.data(), res.size() );
    nr++;    
  }
}
void Room::registerVote( bool voteUp ){
  if ( !m_openMapVote )
    return;
  m_mapVotesMissing--;
  m_mapVote += voteUp ? 1 : -1;
  if ( m_mapVotesMissing == 0 )
    voteMapFinal();
}
void Room::voteMapFinal() {
  if ( !m_openMapVote )
    return;
  m_openMapVote = false;
  if ( m_mapVote >= 0 ) {
    Lvl *l = g_lvlMgr->getLvl(m_voteForMapID);
    if ( !l->valid() )
      return;
    m_lvl = l;
    publishNewMap();

    if ( m_clients.size() < l->maxPlayer() )
      informRoomAboutNewClient( nullptr );
    else {
      while ( m_clients.size() > l->maxPlayer() ){
        auto last = m_clients.at(m_clients.size()-1);
        std::wstring kick{L"Kicked"};
        last->sendReply(oc_text, kick.data(), kick.size());
        leaveRoom(last);
      }
    }
  }
  else {
    GameSession systemUser(L"EDDY(svr)");
    addChatMsg( &systemUser, L"Die Karte wurde abgelehnt");
  }
}
void Room::publishPlayerData( GameSession *user, std::wstring msg ) {
  if (m_status != InGame) // keine Statis verteilen, wenn das Spiel nicht aktiv ist.
    return;
  int nr{0};
  for ( auto c : m_clients ) {
    nr++;
    if ( user == c )
      break;
  }
  std::wstring res(L"P_upd:{ \"id\":");
  res += std::to_wstring(nr) + L", \"data\":";
  res += msg;
  res += L"}";

  sendToAll( res, user );
  updateGamePlay();
}
void Room::publishNewMap( ) {
  sendToAll( L"NewMap:" + m_lvl->asJson() );
}
void Room::updateGamePlay() {
  if ( m_status == Status::InGame ) {
    // collect data
    GameSession *seeker{nullptr};
    bool seekerSurvive{true};
    uint fallenPlayers{0};
    uint disabledPlayers{0};
    for ( auto c : m_clients ){
      if ( c->m_prefSeeker == true ) {
        seeker = c;
        seekerSurvive = (c->m_health > 0);
      }
      else {
        fallenPlayers += c->m_died;
        disabledPlayers += c->m_catched || c->m_died;
      }
    }
    // collision Logic  
    int nr{-1};
    for ( auto c : m_clients ) {
      nr++;
      if ( c->m_catched || c->m_died || c == seeker || c->m_hidden )
        continue;

      bool died = (c->m_health <= 0); // with additional var, to prefent hacking
      if ( !c->m_died && died ) {
        using namespace std::chrono;
        c->m_diedTime = duration_cast<seconds>(high_resolution_clock::now() - m_startTime).count() - 3;
        c->m_died= true;
        fallenPlayers++;
        disabledPlayers++;
        sendToAll( std::wstring(L"GP_upd:{ \"type\":\"Died\", \"nr\":") + std::to_wstring( nr ) + L", \"name\":\"" + c->m_username + L"\" }");
        continue;
      }

      if ( !c->m_x && !c->m_y && !c->m_z) // stil on startposition all 0
        continue;
      bool catched = hypot3( seeker->m_x - c->m_x, seeker->m_y - c->m_y, seeker->m_z - c->m_z) < 1.5f;
      //Blickrichtung interieren
      if ( !c->m_died && !c->m_catched && catched ) {
        using namespace std::chrono;
        c->m_catchTime = duration_cast<seconds>(high_resolution_clock::now() - m_startTime).count() - 3;
        c->m_catched = true;
        g_userMgr->changeRepu(seeker, 1);
        disabledPlayers++;
        sendToAll( std::wstring(L"GP_upd:{ \"type\":\"Catched\", \"nr\":") + std::to_wstring( nr ) + L", \"name\":\"" + c->m_username + L"\" }");
      }      
    }

    // logic about changeing gamestate
    if ( !seekerSurvive ) { // seeker died
      sendToAll(L"GP_upd:{ \"type\":\"PlayerWins\", \"reason\":1 }");
      setGameEnd( PlayerType::Player, GameEnd::Died );
    }
    else if ( fallenPlayers >= m_clients.size() - 1 ) { // all died
      sendToAll(L"GP_upd:{ \"type\":\"SeekerWins\", \"reason\":1 }");
      setGameEnd( PlayerType::Seeker, GameEnd::Died );
    }     
    else if ( disabledPlayers >= m_clients.size() - 1 ) {  // all catched
      sendToAll(L"GP_upd:{ \"type\":\"SeekerWins\", \"reason\":0 }");
      setGameEnd( PlayerType::Seeker, GameEnd::Catched );
    }
  }
  if ( m_status == Status::Finish ) {
    for ( auto c : m_clients ) 
      g_userMgr->addExp( c, 1 );
    g_gameSvr->setTimer( 3000, [this](uint){ publishResult(); }, true );
  }
}
void Room::publishResult() {
  std::wstring res;
  std::wostringstream ss(res);
  ss << L"GP_result:{";
  if ( m_winner == PlayerType::Seeker )
    ss << L"\"winner\":\"Seeker\"";
  else
    ss << L"\"winner\":\"Players\"";
  switch ( m_gameEndReason ) {
  case GameEnd::Catched:   ss << L",\"reason\":\"Catched\""; break;
  case GameEnd::Died:      ss << L",\"reason\":\"Died\""; break;
  case GameEnd::Left:      ss << L",\"reason\":\"Left\""; break;
  case GameEnd::TimeOver:  ss << L",\"reason\":\"TimeOver\""; break;
  }
  for ( auto c: m_clients )
    if (c->m_prefSeeker) {
      ss << L",\"seeker\":{";
      ss << L"\"name\":\"" << c->m_username;
      ss << L"\",\"health\":" << c->m_health;
      ss << L"}";
    }
  for ( auto lP : m_leftPlayers)
    if ( lP->m_wasSeeker ) 
      ss << L",\"seeker\":{\"name\":\"" << lP->m_username << L"\",\"health\":0 }" ;

  ss << L",\"player\":[";  
  bool first{true};  
  for ( auto c: m_clients ) {
    if ( !c->m_prefSeeker) {
      if ( !first )
        ss << L",";
      first = false;
      ss << L"{\"health\":" << c->m_health;
      ss << L",\"name\":\"" << c->m_username;
      ss << L"\",\"catched\":" << c->m_catched;
      ss << L",\"catchTime\":" << c->m_catchTime;
      ss << L",\"left\":false,\"died\":" << c->m_died;
      ss << L",\"diedTime\":" << c->m_diedTime;
      ss << L"}";
    }
  }
  for ( auto lP : m_leftPlayers)
    if ( !lP->m_wasSeeker ){
      if ( !first )
        ss << L",";
      first = false;
      if (lP->m_wasDath)
        ss << L"{\"health\":0";
      else
        ss << L"{\"health\":100";
      ss << L",\"name\":\"" << lP->m_username;
      ss << L"\",\"catched\":" << lP->m_wasCatched;
      ss << L",\"catchTime\":" << lP->m_DiedOrCatchedAfterSeconds;
      ss << L",\"left\":true,\"died\":" << lP->m_wasDath;
      ss << L",\"diedTime\":" << lP->m_DiedOrCatchedAfterSeconds;
      ss << L"}";
    }
  ss << L"]";


  ss << L"}";
  sendToAll(ss.str());

  for ( auto c: m_clients ) {
    c->m_prefSeeker = false;
    c->m_ready = false;
    c->m_catched = false;
    c->m_died = false;
  }
  informRoomAboutNewClient( nullptr );
  for ( auto lP : m_leftPlayers)
    delete lP;
  m_leftPlayers.clear();
}

void Room::startGameTimer() {
  m_status = Status::StartUp;
  GameSession systemUser(L"EDDY(svr)");
  if ( m_startGameTimer == 0 ) {
    addChatMsg( &systemUser, L"Spiel Startet in:" );
    addChatMsg( &systemUser, L"5" );
  } 
  else {
    g_gameSvr->stopTimer(m_startGameTimer);
    m_startGameTimer = 0;
    addChatMsg( &systemUser, L"Unsicher? nochmal:" );
    addChatMsg( &systemUser, L"5" );
  }
  m_startGame = 5;
  m_startGameTimer = g_gameSvr->setTimer( 1000, [this](uint){
    m_startGame--;
    GameSession systemUser(L"EDDY(svr)");
    addChatMsg( &systemUser, std::to_wstring(m_startGame) );   
    if ( m_startGame == 0 ) {
      g_gameSvr->stopTimer(m_startGameTimer);
      m_startGameTimer = 0;
      startGame();
    }
  } );
}
void Room::startGame() {
  stopGameTimer();
  if ( m_clients.size() <= 1 ) // alle zuvor schon abgehauen!!
    return;
  fixSeeker();
  for ( auto c : m_clients ) 
    c->m_inGame = true;
  
  sendWebGlInitPackage();
}
void Room::playerReady() {
  if ( m_status != Status::StartUp )
    return;

  uint playersReady{0};
  for ( auto c : m_clients )
    playersReady += c->m_loadGameDone ? 1 : 0;
  

  if ( playersReady >= m_clients.size() ) {
    std::vector<int> startPosNr;
    for (uint i = 0; i < m_clients.size(); ++i)
      startPosNr.push_back(i);

    std::wstring data{ L"{ \"time\":" };
    data += std::to_wstring(m_gameDuration);
    data += L", \"skill\":[";
    bool first{true};
    uint nr{0};
    for ( auto c : m_clients ){
      nr++;
      if ( c->m_skillData.size() == 0)
        continue;
      if (!first )
        data += L",";
      else
        first = false;

      auto pos = startPosNr.begin()+(rand() % startPosNr.size());
      uint startPos = *pos;
      startPosNr.erase(pos);

      data += L"{\"id\": " + std::to_wstring( nr );
      data += L", \"startPos\": " + std::to_wstring( startPos );
      data += L", \"data\":" + c->m_skillData + L"}";
    }
    data += L"]}";

    m_startTime = std::chrono::high_resolution_clock::now();

    setupGameTimeout();
    sendToAll( L"LOS:"+data );
    g_gameSvr->setTimer( 3000, [this](uint){m_status = Status::InGame;}, true);
  }
}

void Room::setupGameTimeout() {
  GameSession systemUser(L"EDDY(svr)");
  addChatMsg( &systemUser, L"Los gehts!" );

  m_gameDuration = (5*60 + m_clients.size()*20 - 40 + 3) * 1000;
  m_gameTimer = g_gameSvr->setTimer( m_gameDuration-(60*1000), [this](uint timer){
    GameSession systemUser(L"EDDY(svr)");
    addChatMsg( &systemUser, L"noch 1 Minute" );
    sendToAll( L"1MinLeft" );
    m_gameTimer = g_gameSvr->setTimer( 30*1000, [this](uint timer){
      GameSession systemUser(L"EDDY(svr)");
      addChatMsg( &systemUser, L"noch 30 Secunden" );
      m_gameTimer = g_gameSvr->setTimer( 20*1000, [this](uint timer){
        GameSession systemUser(L"EDDY(svr)");
        addChatMsg( &systemUser, L"noch 10 Secunden" );
        sendToAll( L"10SecLeft" );
        m_gameTimer = g_gameSvr->setTimer( 7*1000, [this](uint timer){
          GameSession systemUser(L"EDDY(svr)");
          addChatMsg( &systemUser, L"noch 3 Secunden" );
          m_gameTimer = g_gameSvr->setTimer( 1*1000, [this](uint timer){
            GameSession systemUser(L"EDDY(svr)");
            addChatMsg( &systemUser, L"noch 2 Secunden" );
            m_gameTimer = g_gameSvr->setTimer( 1*1000, [this](uint timer){
              GameSession systemUser(L"EDDY(svr)");
              addChatMsg( &systemUser, L"noch 1 Secunden" );
              m_gameTimer = g_gameSvr->setTimer( 1*1000, [this](uint timer){
                m_gameTimer = 0;
                GameSession systemUser(L"EDDY(svr)");
                addChatMsg( &systemUser, L"STOP!" );
                sendToAll( L"TimeOver" );
                sendToAll(L"GP_upd:{ \"type\":\"PlayerWins\", \"reason\":3 }");
                setGameEnd( PlayerType::Player, GameEnd::TimeOver );
                updateGamePlay();
              }, true);
            }, true);
          }, true);
        }, true);
      }, true);
    }, true);
  }, true);
}
void Room::stopGameTimer() {
  if ( m_gameTimer )
    g_gameSvr->stopTimer(m_gameTimer);
  m_gameTimer = 0;
}
void Room::setGameEnd( PlayerType winner, GameEnd reason ) {
  stopGameTimer();
  m_winner = winner;
  m_gameEndReason = reason;
  m_status = Finish;
}
void Room::fixSeeker() {
  uint prefSeeker{0};
  for ( auto c : m_clients ) 
    prefSeeker += (c->m_prefSeeker ? 1 : 0);
  
  std::cout << prefSeeker  << std::endl;
  if ( prefSeeker == 1 )
    return;
  
  if ( prefSeeker > 1 ) {
    uint seeker = rand() % prefSeeker;
    uint counter{0};
    for ( auto c : m_clients ) 
      if ( c->m_prefSeeker ) 
        c->m_prefSeeker = seeker == counter++ ? true : false;
    return;
  }

  uint seeker = rand() % m_clients.size();
  uint counter{0};
  for ( auto c : m_clients ) 
    c->m_prefSeeker = seeker == counter++ ? true : false;
}
void Room::informRoomAboutNewClient( GameSession* newClient ) {
  std::wstring res(L"Players:");
  res.append( playersJson() );

  for ( auto c : m_clients )
    if ( c != newClient )
      c->sendReply( oc_text, res.c_str(), res.size() );
}
void Room::informRoomAboutNewStat( ) {
  std::wstring res(L"Stats:");
  res.append( playerStatsJson() );

  for ( auto c : m_clients )
    c->sendReply( oc_text, res.c_str(), res.size() );
}

void Room::sendWebGlInitPackage() { // jeder client bekommt ein indivituelle paket
  for ( auto c : m_clients ){
    std::wstring res(L"StartGame:[");
    bool first{true};
    uint nr{0};
    for ( GameSession* s : m_clients ) {
      if ( !first )
        res += L",";
      first = false;
      res += L"{\"nr\":" + std::to_wstring(nr) + L", ";
      res += L"\"seek\":";
      res += ( s->m_prefSeeker ? L"1," : L"0," );
      res += L"\"local\":";
      res += ( c == s ? L"1" : L"0" );
      res += L"}";
      nr++;  
    }
    res += L"]";
    c->sendReply( oc_text, res.c_str(), res.size() );
  }
}
void Room::sendToAll( std::wstring msg, GameSession *exept ) {
  for ( auto c : m_clients )
    if ( c != exept )
      c->sendReply( oc_text, msg.data(), msg.size() );
}