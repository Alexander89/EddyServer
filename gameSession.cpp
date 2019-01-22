#include "gameSession.h"
#include "gameServer.h"
#include "userMgr.h"
#include "lvlMgr.h"
#include "helperfunctions.h"
#include "json.h"
#include <iostream>
#include <locale>
#include <string>
#include <vector>
#include <codecvt>
#include <fstream>

using namespace std;

GameSession::GameSession( int socket ) :
  WebSession( socket )
{
  start();
}
GameSession::GameSession( std::wstring username ) :
  WebSession( 0 ),
  m_username(username)
{
}
GameSession::GameSession( ) :
  WebSession( 0 )
{
}
GameSession::~GameSession() {

}


void GameSession::handleContinuationData( char* data, int lng ) {
  cout << "get stream data, lng:" << lng << endl;  
}
void GameSession::handleTextData( char* data, int lng ) {
  string request(data, lng);
  if ( request == "getRoomList") {
    std::wstring res(L"RoomList:");
    g_gameSvr->leaveAllRooms(this);
    res.append( g_gameSvr->getRoomJSONList());
    sendReply( oc_text, res.c_str(), res.size() );
  }
  else if ( request.substr(0,8) == "getRoom/") { try {
    int room = stoi(request.substr(8)); 
    std::wstring res;
    if ( g_gameSvr->joinRoom(this, room) ){
      res = L"Room:";
      res.append( g_gameSvr->getRoomJSON(room));  
    }
    else
      res = L"RoomFull";
    sendReply( oc_text, res.c_str(), res.size() );
  }catch( ... ){};}
  else if ( request.substr(0,8)== "getMaps/") { try {
    int start = stoi(request.substr(8)); 
    std::wstring res{L"MapSelect:"};
    res.append( g_lvlMgr->getJsonMapSelect(start, start + 20) );  
    sendReply( oc_text, res.c_str(), res.size() );
  }catch( ... ){};}
  else if ( request.substr(0,14)== "MapSuggestion/") { try {
    int mapID = stoi(request.substr(14));
    g_gameSvr->mapSuggestion(this, mapID);;
  }catch( ... ){};}
  else if ( request == "leftResult") {
    m_inGame = false;
    g_gameSvr->leftResult(this);    
  }
  else if ( request == "VoteMapUp")
    g_gameSvr->voteMap(this, true);    
  else if ( request == "VoteMapDown")
    g_gameSvr->voteMap(this, false);    
  else if ( request == "ready")
    g_gameSvr->setReady(this);
  else if ( request == "readySeeker")
    g_gameSvr->setReadySeeker(this);
  else if ( request == "loadGameDone") {
    m_loadGameDone = true;
    g_gameSvr->setLoadGameReady(this);
  }
  else if ( request.substr(0,6) == "login/") {
    std::wstring passwd;
    auto reqData = splitString( request.substr(6), "/" );
    for ( auto line: *reqData ) {
      auto fields = splitString( line , ":" );
      if ( fields->size() == 2 ) {
        std::string key = fields->at(0);
        if ( key == "name" ){
          std::wstring_convert< std::codecvt_utf8<wchar_t> > converter;
          m_username = converter.from_bytes(fields->at(1).data());
        }
        else if ( key == "pass" ){
          std::wstring_convert< std::codecvt_utf8<wchar_t> > converter;
          passwd = converter.from_bytes(fields->at(1).data());
        }
      }
      delete fields;
    }
    delete reqData;

    g_userMgr->bindUserdata( this, passwd );
    std::wstring res(L"Login:");
    res += L"{\"login\":1, \"username\":\"" + m_username + L"\"}";
    std::wcout << res << std::endl;  
    sendReply( oc_text, res.c_str(), res.size() );
  }
  else if ( request == "logoff/") {
    m_username = L"";
    m_reputation = 0;
    m_exp = 0;  
    const wchar_t res[] = L"Login:{\"login\":0 }";
    sendReply( oc_text, res, 18 );
  }
  else if ( request.substr(0,5) == "chat/") {
    std::wstring_convert< std::codecvt_utf8<wchar_t> > converter;
    g_gameSvr->addChatMessage( this, converter.from_bytes( request.substr(5).data() ) );
  }
  else if ( request.substr(0,12) == "skillValues/") {
    std::wstring_convert< std::codecvt_utf8<wchar_t> > converter;
    m_skillData = converter.from_bytes( request.substr(12).data() );
  }
  else if ( request.substr(0,11) == "PlayerData/") {
    std::wstring_convert< std::codecvt_utf8<wchar_t> > converter;
    std::wstring playerInfo = converter.from_bytes( request.substr(11).data() );
    JSON info(playerInfo);

    m_x = info.get(L"pos", L"0")->toFloat();
    m_y = info.get(L"pos", L"2")->toFloat();
    m_z = info.get(L"pos", L"1")->toFloat();

    m_rotZ = info.get(L"rot", L"1")->toFloat();
    m_health = info.get(L"he")->toInt();
    m_hidden = info.get(L"h")->toBool();

    g_gameSvr->publishPlayerData( this, playerInfo );
  }
  else if ( request.substr(0,9) == "Feedback/") {
    std::string name{"feedback/"};
    char username[100]{0};
    wcstombs(username, m_username.data(), m_username.size());
    name += username;
    name += "_" + std::to_string(m_userID) + ".txt";

    std::ofstream out (name, std::ofstream::out | std::ofstream::app);
    out << time(nullptr) << std::endl;
    out << request.substr(9)<< std::endl;
    out << "-----------------------------------------------------"<< std::endl;
    out.close();
  }
}

void GameSession::handleBinaryData( char* data, int lng ) {
  cout << "get binary data, lng:" << lng << endl;  
}

void GameSession::clientDisconnected() {
  cout << "client closed session "<< endl;  
  g_gameSvr->clientDisconnected(this);
}

std::wstring GameSession::getPlayerJSON() {
  std::wstring res;
  res += L"{";
  res += L"\"name\":\"" + m_username + L"\",";
  res += L"\"rep\":" + std::to_wstring( m_reputation ) + L",";
  res += L"\"exp\":" + std::to_wstring( m_exp ) + L",";
  res += L"\"stat\":\"";
    if ( m_ready && m_prefSeeker )
      res += L"Seek";
    else if ( m_ready )
      res += L"Ready";
    else
      res += L"Open";
  res += L"\"}";
  return res;
}

void GameSession::resetPlayer() {
  if ( this == 0 )
    return;
   m_ready = false;
   m_prefSeeker = false;

   m_loadGameDone = false;
   
   m_x = 0.f;
   m_y = 0.f;
   m_z = 0.f;
   m_rotZ = 0.f;
   m_health = 100;
   m_catched = false;
   m_died = false;
   m_hidden = false;
   m_catchTime = 0.f;
   m_diedTime = 0.f;
}