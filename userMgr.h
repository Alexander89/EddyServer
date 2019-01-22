#ifndef USERMGR_H
#define USERMGR_H
#include <map>
#include <string>
#include "lvl.h"

struct User {
  uint m_userID{0};
  std::wstring m_username;
  std::wstring m_password;
  int m_reputation{0};
  uint m_exp{0};
};
class GameSession;

class UserMgr
{
public:
  UserMgr();
  ~UserMgr();

  void bindUserdata( GameSession *s, std::wstring passwd );
  void createNewUser( GameSession *s, std::wstring passwd );
  void changeRepu( GameSession *s, short repuChange );
  void addExp( GameSession *s, short count );
  
private:
  void readUserDB();
  void saveUserDB();

private:
  std::map< std::pair<std::wstring, std::wstring> , User*> m_users;
  std::map< uint, User*> m_usersByID;

  uint m_nextUserID{0};
  bool m_dbChanged{false};

};

extern UserMgr *g_userMgr;

#endif // USERMGR_H