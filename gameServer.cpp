#include "gameServer.h"

#include "userMgr.h"
#include "room.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <time.h> 


GameServer *g_gameSvr{nullptr};
void* timerThreadStart( void* obj );

GameServer::GameServer( ) :
  m_server( 17516 )
{
  srand( time(0) );
  g_gameSvr = this;
  new UserMgr();
  
  std::cout << "init gameServer" << std::endl;
  createNewRoom();

  std::cout << "init Timer" << std::endl;
  pthread_create( &m_timerThreadID, nullptr, timerThreadStart, this );  
}
GameServer::~GameServer(){
  g_gameSvr = nullptr;
}
void GameServer::start() {
  m_server.start();
}

void GameServer::stop() {
 m_server.stop(); 
}

Room *GameServer::getRoom(uint id) {
  for ( Room *room : m_rooms)
    if ( room->id() == id )
      return room;
  return nullptr;
}
Room *GameServer::createNewRoom() {
  Room *room = new Room();
  m_rooms.push_back(room);
  return room;
}

void GameServer::clientDisconnected( GameSession *user ) {
  leaveAllRooms(user);
  // .. leave game.....
}
bool GameServer::joinRoom( GameSession *user, int roomId ) {
  Room *room = getRoom(roomId);
  if ( !room )
    return false;
  leaveAllRooms(user);
  if ( room->status() == Status::InGame || room->status() == Status::StartUp )
    return false;

  if ( room->joinRoom( user ) ){
    std::cout << " user joined the room " << std::endl;
    m_assignedSessions.emplace(user->pid(), room);

    if ( !isEmptyRoomAvailable() )
      createNewRoom();
    return true;
  }
  std::cout << " user cant join the room" << std::endl;
  return false;
}

void GameServer::leaveAllRooms( GameSession *user ) {
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  std::cout << " found user so remove it from the room " << std::endl;
  (active->second)->leaveRoom(user);
  m_assignedSessions.erase(active);
  keepFirstEmptyRoom();
}

void GameServer::addChatMessage( GameSession *user, std::wstring msg ){
  std::wcout << msg << std::endl;
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  (active->second)->addChatMsg(user, msg);    
}
void GameServer::mapSuggestion( GameSession *user, uint mapID ){
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  (active->second)->mapSuggestion(user, mapID);    
}
void GameServer::voteMap( GameSession *user, bool up ){
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  (active->second)->registerVote(up);  
}
void GameServer::publishPlayerData( GameSession *user, std::wstring msg ){
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  (active->second)->publishPlayerData(user, msg);  
}
void GameServer::leftResult( GameSession *user ) {
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  (active->second)->leftResult(user);
}
void GameServer::setReady( GameSession *user ) {
  if ( user->m_ready == true && user->m_prefSeeker == false)
    return;
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  user->m_ready = true;
  user->m_prefSeeker = false;
  (active->second)->updateStat(user);
}
void GameServer::setReadySeeker( GameSession *user ){
  if ( user->m_ready == true && user->m_prefSeeker == true)
    return;
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  user->m_ready = true;
  user->m_prefSeeker = true;
  (active->second)->updateStat(user);
}
void GameServer::setLoadGameReady( GameSession *user ){
  auto active = m_assignedSessions.find(user->pid());
  if ( active == m_assignedSessions.end() )
    return;
  (active->second)->playerReady();
}
bool GameServer::isEmptyRoomAvailable() {
  for ( Room *room : m_rooms)
    if ( room->isEmpty() )
      return true;
  return false;
}
void GameServer::keepFirstEmptyRoom() {
  bool emptyFound{false};
  for ( auto i = m_rooms.begin(); i != m_rooms.end(); ++i ) {
    Room *room = (*i);
    if ( room->isEmpty() ){
      if ( !emptyFound )
        emptyFound = true;
      else {
        m_rooms.erase(i);
        emptyFound = false;

        delete room;
        i = m_rooms.begin();
      }
    }
  }
}

std::vector< std::vector<std::wstring> > GameServer::getRoomList() {
  std::vector< std::vector<std::wstring> > res;

  for ( auto room : m_rooms ) {
    std::vector<std::wstring> roomData;
    roomData.push_back( room->name() );
    roomData.push_back( std::to_wstring( room->connections() ) );
    res.push_back(roomData);
  }
  return res;
}

std::wstring GameServer::getRoomJSONList() {
  std::wstring res{L"["};
  bool first{true};
  for ( auto room : m_rooms ) {
    if ( !first )
      res += L",";
    first = false;
    res += room->getJsonOverview();
  }
  res += L"]";
  return res;
}
std::wstring GameServer::getRoomJSON(uint id) {
  auto room = getRoom(id);
  if ( !room )
    return L"";
  return room->getJson();
}

void* timerThreadStart( void* obj ) {
  static_cast<GameServer*>( obj )->timerThread();
  return nullptr;
}


void GameServer::timerThread() {
  while( !m_terminate ) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    auto now = std::chrono::high_resolution_clock::now();

    for( auto p : m_timers ){
      auto timer = p.second;
      auto duration = now - timer->m_startTime;
      uint ticks = std::chrono::duration_cast<std::chrono::duration<int, std::milli> >(duration).count();
      if ( ticks > timer->m_interval * timer->m_counter ) {
        ++timer->m_counter;
        timer->m_callback(timer->m_id);
        if ( timer->m_singleShot )
          stopTimer(timer->m_id);
      }
    }
  }
}
uint GameServer::setTimer( uint timeout, std::function<void(uint)> callback, bool singleShot ) {
  timerEntry *timer = new timerEntry();
  timer->m_id = m_lastTimerID++;
  timer->m_interval = timeout;
  timer->m_singleShot = singleShot;
  timer->m_startTime = std::chrono::high_resolution_clock::now();
  timer->m_callback = callback;  
  std::cout << "make Timer " << timer->m_id << std::endl;
  m_timers.emplace( timer->m_id, timer );
  return timer->m_id;
}
void GameServer::stopTimer( uint timerID ) {
  auto itr = m_timers.find(timerID);
  if ( itr == m_timers.end() )
    return;
  
  std::cout << "killtimer " <<timerID << std::endl;
  auto timerEntry = (*itr).second;
  m_timers.erase(itr);
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(10us);
  delete timerEntry;
}