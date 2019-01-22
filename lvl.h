#ifndef LVL_H
#define LVL_H

#include <string>

class Lvl
{
public:
  explicit Lvl( uint lvlID );
  ~Lvl();

  uint maxPlayer() const { return m_maxPlayer; }

  bool valid() const { return !m_fileInvalid; }
  std::wstring asJsonNoData() const;
  std::wstring asJson() const;

  const char *getLvlAsBase64( uint *size = nullptr ) const;

private:
  void readMetaFile( const std::string &metaFilePath );
  void writeMetaFile( const std::string &metaFilePath );
  void readBmpFile( const std::string &lvlname );
  
private:
  uint         m_id{0};
  std::wstring m_name;
  uint         m_maxPlayer{0};
  std::wstring m_creator;
  std::wstring m_createDate;
  std::wstring m_version;
  std::wstring m_beschreibung;
  std::string  m_imgPath;
  char*        m_img{nullptr};
  bool         m_fileInvalid{true};
  uint         m_imgSize{0};

};

#endif // LVL_H