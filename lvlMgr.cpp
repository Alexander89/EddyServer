#include "lvlMgr.h"
#include <iostream>
#include <sys/types.h>
#include <dirent.h>

LvlMgr *g_lvlMgr{nullptr};

LvlMgr::LvlMgr() {
  g_lvlMgr = this;
  readAllMaps();
}
LvlMgr::~LvlMgr() {
  for ( auto l : m_levels)
    delete l.second; 
  m_levels.clear();
}

void LvlMgr::readAllMaps() {  
  DIR *dp = opendir("lvl");
  if( !dp ) {
    std::cout << "Error opening dir ./lvl " << std::endl;
    return;
  }

  struct dirent *dir;
  while ( (dir = readdir(dp)) ) {
    std::string filename(dir->d_name);
    if ( filename.substr(0,5) != "level" )
      continue;
    if ( filename.substr( filename.size()-4,4) == ".bmp" )
      continue;
    try {
      uint id = stoi( filename.substr(5,7) );
      getLvl(id);
    }
    catch( ... ){};

  }
  closedir(dp);
  return ;
}

Lvl *LvlMgr::getLvl( uint id ){
  if ( this == nullptr )
    return (new LvlMgr())->getLvl(id);
  
  if ( m_levels.count(id) == 0 ) {
    Lvl* l = new Lvl(id);
    m_levels.emplace(id, l);
    return l;
  }
  return m_levels[id];
}

std::wstring LvlMgr::getJsonMapSelect( uint start, uint end ){
  std::wstring res{L"["};
  bool first = true;
  auto itr = m_levels.begin();
  std::advance(itr, start);
  uint counter = start;
  while ( counter < end ) {
    if (!first)
      res += L",";
    first = false;
    res += itr->second->asJson(); 

    ++itr;
    ++counter;
    if ( itr == m_levels.end() )
      break;
  }
  res +=L"]";
  return res;
}