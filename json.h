#ifndef JSON_H
#define JSON_H
#include <vector>
#include <string>
#include <sstream>

enum class TokenType {
  undefined,
  Symbol,
  NumberConst,
  StringConst,
  BoolConst
};

class JSONTokeniser {
public:
  JSONTokeniser() {};
  JSONTokeniser( std::wstring &input );
  ~JSONTokeniser();

  bool nextToken();
  bool peekNextToken();
  TokenType tokenType()const{ return m_tokenType; }
  wchar_t symbol() const    { return m_symbol; }
  float numberVal() const   { return m_number; }
  bool boolVal() const      { return m_bool; }
  std::wstring stringVal() const  { return m_string; }
private:
  bool isSymbol( wchar_t c ) const;

  std::wistringstream  m_input;
  TokenType    m_tokenType{TokenType::undefined};
  wchar_t      m_symbol{0};
  std::wstring m_string;
  bool         m_bool{false};
  double       m_number{0.0};
  
  bool         m_useLastToken{false};
};

enum class JSONDataType {
  Undef,
  Bool,
  Number,
  Text,
  Object,
  Array,
};

struct JSONEntry {
  JSONEntry( JSONEntry *parent, JSONDataType type = JSONDataType::Undef ): m_parent(parent), m_type(type) { if (parent) parent->m_members.push_back(this); };
  ~JSONEntry() { for ( auto m : m_members) delete m; }
  std::wstring arrayIndex() const;
  std::wstring toString( bool asJSON = false) const;
  bool toBool() const;
  int toInt() const;
  long toLong() const;
  float toFloat() const;
  double toDouble() const;

  JSONEntry*   m_parent{nullptr};
  std::wstring m_name;
  JSONDataType m_type{JSONDataType::Undef};
  double       m_nr{0.0};
  bool         m_bool{false};
  std::wstring m_text;
  std::vector<JSONEntry*> m_members;
};

class JSON {
public:
  JSON() : m_root(nullptr, JSONDataType::Object ) {}
  JSON( std::wstring &input ) : m_root(nullptr, JSONDataType::Object ), m_reader(input) { parseText(input); }
  //JSON( &JSON );
  ~JSON() {};

  JSON *parseText( std::wstring &input ); 
  const JSONEntry *getRoot() const { return &m_root; }
  const JSONEntry *get( ) const { return getRoot(); };
  template<typename... Targs>
  const JSONEntry *get( Targs... identifier ) const { return getEntry( getRoot(), identifier... ); }

  const JSONEntry *getEntry( const wchar_t *identifier ) const { return findElement( getRoot(), identifier ); }
  const JSONEntry *getEntry( const JSONEntry* entry, const wchar_t *identifier ) const { return findElement(entry, identifier);  }
  template<typename... Targs>
  const JSONEntry *getEntry( const JSONEntry* entry, const wchar_t *value, Targs... identifier ) const { return getEntry( findElement(entry, value), identifier... ); }

  const JSONEntry *findElement( const JSONEntry *entry, const wchar_t *sub ) const;

private:
  JSONEntry m_root;
  JSONTokeniser m_reader;
};

#endif //JSON
