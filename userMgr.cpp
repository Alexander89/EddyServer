#include "userMgr.h"
#include "gameServer.h"
#include "gameSession.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <locale>
#include <codecvt>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

UserMgr *g_userMgr{nullptr};

UserMgr::UserMgr() {
  g_userMgr = this;
  readUserDB();
  g_gameSvr->setTimer( 30000, [this](uint){ 
    if(m_dbChanged){
      m_dbChanged = false;
      saveUserDB();
    }
  } );
}
UserMgr::~UserMgr() {
  saveUserDB();
  for ( auto l : m_users)
    delete l.second; 
  
  m_users.clear();
  m_usersByID.clear();
}

void UserMgr::bindUserdata( GameSession *s, std::wstring passwd ){
  if ( this == nullptr ) {
    ( new UserMgr() )->bindUserdata(s, passwd);
    return;
  }
  std::pair<std::wstring, std::wstring> key{s->m_username, passwd};

  auto it = m_users.find(key);
  if ( it == m_users.end() )
    createNewUser( s, passwd );
  else {
    User *u = it->second;
    s->m_userID = u->m_userID;
    s->m_reputation = u->m_reputation;
    s->m_exp = u->m_exp;
  }
}

void UserMgr::createNewUser( GameSession *s, std::wstring passwd ) {
  User *u = new User;
  u->m_username = s->m_username;
  u->m_password = passwd;
  u->m_reputation = 0;
  u->m_exp = 0;
  u->m_userID = m_nextUserID++;
  m_users.emplace( std::make_pair(u->m_username, u->m_password), u);
  m_usersByID.emplace( u->m_userID, u );
  m_dbChanged = true;
}


void UserMgr::changeRepu( GameSession *s, short repuChange ) {
  s->m_reputation += repuChange;
  auto itr = m_usersByID.find(s->m_userID);
  if ( itr == m_usersByID.end() ) // schade nicht mehr gefunden, somit krigt er auch keinen schlechten Eintrag
    return;   
  itr->second->m_reputation += repuChange;
  m_dbChanged = true;
}
void UserMgr::addExp( GameSession *s, short count ) {
  s->m_exp += count;
  auto itr = m_usersByID.find(s->m_userID);
  if ( itr == m_usersByID.end() ) // schade nicht mehr gefunden, leider bekommt er dann auch kein + aufs konto
    return;   
  itr->second->m_exp += count;
  m_dbChanged = true;
}

void UserMgr::readUserDB() {
  std::wifstream in( "data/userDB" );
  if (in.is_open()) { // apply BOM-sensitive UTF-16 facet
    in.imbue(std::locale(in.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    while ( in.good() ) {
      User *u = new User;
      in >> u->m_username;
      in >> u->m_password;
      if ( u->m_username.size() && u->m_password.size() ) {
        in >> u->m_reputation;
        in >> u->m_exp;
        u->m_userID = m_nextUserID++;
        m_users.emplace( std::make_pair(u->m_username, u->m_password), u);
        m_usersByID.emplace( u->m_userID, u );
      }
      else {
        delete u;
      }
    }
    in.close();
  }
}

void UserMgr::saveUserDB() {
  std::cout << "write UserDB " << std::endl;
  std::wofstream out( "data/userDB_temp", std::wofstream::out| std::wofstream::trunc );
  if (out.is_open()) { // apply BOM-sensitive UTF-16 facet
    out.imbue(std::locale(out.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    for( auto pair : m_users ) {      
      User *u = pair.second; 
      if ( u->m_username.size() && u->m_password.size() ) {
        out << u->m_username << std::endl;
        out << u->m_password << std::endl;
        out << u->m_reputation << std::endl;
        out << u->m_exp << std::endl;
      }
    }
    out.close();
  }
  std::rename( "data/userDB_temp", "data/userDB" );
}