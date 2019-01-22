#include "lvl.h"
#include "helperfunctions.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


Lvl::Lvl(uint lvlID) {
  std::string metaPath( "lvl/level" );
  metaPath += std::to_string(lvlID); 
  readMetaFile( metaPath );
  readBmpFile( m_imgPath );
}
Lvl::~Lvl() {
  delete[] m_img;
  m_img = nullptr;
  m_imgSize = 0;
}
void Lvl::readMetaFile( const std::string &lvlname ) {
  std::wifstream in( lvlname );
  if (in.is_open()) { // apply BOM-sensitive UTF-16 facet
    in.imbue(std::locale(in.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    std::wstring line;
    while (std::getline(in, line)) {
      std::wstring convert;
      if ( line.find(L"id:") == 0)
        m_id = stoi(line.substr(3));
      if ( line.find(L"name:") == 0)
        m_name = line.substr(5, line.size() - 6);
      if ( line.find(L"desc:") == 0)
        m_beschreibung = line.substr(5, line.size() - 6);
      if ( line.find(L"vers:") == 0)
        m_version = line.substr(5, line.size() - 6);
      if ( line.find(L"maxP:") == 0)
        m_maxPlayer = stoi(line.substr(5));
      if ( line.find(L"auth:") == 0)
        m_creator = line.substr(5, line.size() - 6);
      if ( line.find(L"date:") == 0)
        m_createDate = line.substr(5, line.size() - 6);
    }
    in.close();
  }

  std::wstringstream ss;
  ss << L"lvl/" << m_name << L"_" << m_id << L"_" << m_version << L".bmp"; 

  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  m_imgPath = converter.to_bytes(ss.str());
  
  //std::cout << "res: " << m_imgPath << std::endl;
}
void Lvl::writeMetaFile( const std::string &lvlname ) {
  std::wofstream out( lvlname );
  if (out.is_open()) { // apply BOM-sensitive UTF-16 facet
    out.imbue(std::locale(out.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    out << L"id:" << m_id << std::endl;
    out << L"name:" << m_name << std::endl;
    out << L"desc:" << m_beschreibung << std::endl;
    out << L"vers:" << m_version << std::endl;
    out << L"maxP:" << m_maxPlayer << std::endl;
    out << L"auth:" << m_creator << std::endl;
    out << L"date:" << m_createDate << std::endl;
  }    
  out.close();
}
void Lvl::readBmpFile( const std::string &lvlname ) {
  int lvlFile = open( lvlname.c_str(), O_RDONLY );
  //std::cout << "readImg " << std::endl;
  if ( lvlFile <= 0 ) // fehler
    return;
  umask(0);

  m_imgSize = lseek(lvlFile, 0, SEEK_END);

  lseek(lvlFile, 0, SEEK_SET);
  m_img = new char[m_imgSize+2]{0};
  if ( read( lvlFile, m_img, m_imgSize ) == m_imgSize )
    m_fileInvalid = false;
  //std::cout << "readImg done " << m_imgSize << std::endl;
}
std::wstring Lvl::asJsonNoData() const {
  std::wstring res;
  res += L"{\"name\":\"" + m_name + L"\",";
  res += L"\"id\":" + std::to_wstring(m_id) + L",";
  res += L"\"maxPlayers\":" + std::to_wstring(m_maxPlayer) + L",";
  res += L"\"creator\":\"" + m_creator + L"\",";
  res += L"\"createDate\":\"" + m_createDate + L"\",";
  res += L"\"version\":\"" + m_version + L"\",";
  res += L"\"beschreibung\":\"" + m_beschreibung + L"\"}";
  return res;
}
std::wstring Lvl::asJson() const {
  std::wstring res;
  res += L"{\"name\":\"" + m_name + L"\",";
  res += L"\"id\":" + std::to_wstring(m_id) + L",";
  res += L"\"maxPlayers\":" + std::to_wstring(m_maxPlayer) + L",";
  res += L"\"creator\":\"" + m_creator + L"\",";
  res += L"\"createDate\":\"" + m_createDate + L"\",";
  res += L"\"version\":\"" + m_version + L"\",";
  res += L"\"beschreibung\":\"" + m_beschreibung + L"\",";
  uint size{0};
  const char *data = getLvlAsBase64(&size);
  std::vector<char> v(data, data+size);
  res += L"\"imgDataBase64\":\"" + std::wstring(v.begin(), v.end()) + L"\"}";
  delete[] data;

  return res;
}
const char *Lvl::getLvlAsBase64( uint *size ) const {
  return toBase64( (unsigned char*)m_img, m_imgSize, size );
}