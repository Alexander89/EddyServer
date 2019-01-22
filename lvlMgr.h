#ifndef LVLMGR_H
#define LVLMGR_H
#include <map>
#include <string.h>
#include "lvl.h"

class LvlMgr
{
public:
  LvlMgr();
  ~LvlMgr();

  void readAllMaps();
  Lvl *getLvl( uint id );
  std::wstring getJsonMapSelect( uint start, uint end );
  
private:
  std::map<uint, Lvl*> m_levels;
};

extern LvlMgr *g_lvlMgr;

#endif // LVLMGR_H