#include "json.h"
#include <iostream>
#include "math.h"

JSONTokeniser::JSONTokeniser( std::wstring &input ) : 
  m_input(input) {
}
JSONTokeniser::~JSONTokeniser() {
}
bool JSONTokeniser::isSymbol( wchar_t c ) const {
  wchar_t symbols[] = L"{[]},:";
  for ( uint i = 0; i < sizeof(symbols)/sizeof(wchar_t); ++i )
    if ( symbols[i] == c )
      return true;
  return false;
}
bool JSONTokeniser::nextToken() {
  if ( m_useLastToken ) {
    m_useLastToken = false;
    return true;
  }
  std::wstring token;
  wchar_t ch;

  m_input >> std::skipws;  
  if( !m_input.good() ) // endoffile
    return false;           

  if (!(m_input >> ch))   // get char
    return false;           // end of file!

  m_input >> std::noskipws;
  if ( isSymbol(ch) ) {   // symbols ar always one field
    m_symbol = ch;
    m_tokenType = TokenType::Symbol;
    return true;
  }
  else if ( ch == '\"') {
    m_tokenType = TokenType::StringConst;
    while (m_input >> ch) {
      if ( ch == '\"') {
        m_string = token;
        return true;
      }
      token += ch; // do this on the end to skip the closing \" (defined)
    }
    return false; // error no end \"
  }
  else if ( std::isdigit(ch) || ch == '-') { // numbers start with an digit and ends with an other letter. do not whiteskip spaces
    token += ch;
    bool e{false};
    while (m_input >> ch) {  //read next char
      if ( e ) {
        if ( std::isdigit(ch) || ch == '-')
          token += ch;
        else {
          m_number *= pow(10,std::wcstod(token.data(), nullptr));
          m_input.putback(ch);
          break;
        }
      }
      else if( ch == 'e' || ch == 'E' ) {
        m_number = std::wcstod(token.data(), nullptr);
        token = std::wstring();
        e = true;
      }
      else if( std::isdigit(ch) || ch == '.' ) 
        token += ch;      
      else {
        m_number = std::wcstod(token.data(), nullptr);
        m_input.putback(ch);
        break;
      }
    }
    m_tokenType = TokenType::NumberConst;
    return true;
  }
  else if ( ch == 'f' || ch == 't' ) { // false or true
    if ( ch == 'f' ){
      if (!(((m_input >> ch), ch == 'a') && ((m_input >> ch), ch == 'l') && ((m_input >> ch), ch == 's') && ((m_input >> ch), ch == 'e')) ){
        m_input.putback(ch);
        return false;
      }
      m_bool = false;
    }
    else {
      if (!(((m_input >> ch), ch == 'r') && ((m_input >> ch), ch == 'u') && ((m_input >> ch), ch == 'e')) ){
        m_input.putback(ch);
        return false;
      }
      m_bool = true;
    }
    m_tokenType = TokenType::BoolConst;
    return true;
  }
  std::cout << "unexpected charecter: " << (char)ch << std::endl;
  std::wcout << L"data:" << m_input.str() << std::endl;
  return false;
}
bool JSONTokeniser::peekNextToken() {
  bool res = nextToken();
  m_useLastToken = true;
  return res;
}

std::wstring JSONEntry::arrayIndex() const { 
  uint nr = m_members.size()-1;
  //if ( nr == 0 )
  //  return L"0";
  return std::to_wstring(nr); 
}
std::wstring JSONEntry::toString( bool asJSON ) const {
  if ( this == nullptr )
    return L"";

  switch( m_type ) {
    case JSONDataType::Undef:
      return L"Undef";
    case JSONDataType::Bool:
      if ( m_bool )
        return L"true";
      return L"false";
    case JSONDataType::Number:
      return std::to_wstring(m_nr);
    case JSONDataType::Text:
      if ( asJSON )
        return L"\"" + m_text + L"\"";
      return m_text;
    case JSONDataType::Object:{
      std::wstring res(L"{");
      bool first{true};
      for( auto e : m_members ) {
        if ( !first )
          res += L",\"";
        else
          res += L"\"";
        first = false;
        res += e->m_name;
        res += L"\":";
        res += e->toString( true );
      }
      res+=L"}";
      return res;
    }
    case JSONDataType::Array: {
      std::wstring res(L"[");
      bool first{true};
      for( auto e : m_members ) {
        if ( !first )
          res += L",";
        first = false;
        res += e->toString( true );
      }
      res+=L"]";
      return res;
    }
  }
  return L"";
}
bool JSONEntry::toBool() const {
  if ( this == nullptr )
    return false;
  switch( m_type ) {
    case JSONDataType::Undef:
      return false;
    case JSONDataType::Bool:
      return m_bool;
    case JSONDataType::Number:
      return m_nr;
    case JSONDataType::Text:
      return m_text.size();
    case JSONDataType::Object:
    case JSONDataType::Array: 
      return m_members.size();
  }
  return false;
}
int JSONEntry::toInt() const {
  if ( this == nullptr )
    return 0;
  if ( m_type == JSONDataType::Number)
    return (int)m_nr;
  return 0;
}
long JSONEntry::toLong() const {
  if ( this == nullptr )
    return 0L;
  if ( m_type == JSONDataType::Number)
    return (long)m_nr;
  return 0L;
}
float JSONEntry::toFloat() const {
  if ( this == nullptr )
    return 0.f;
  if ( m_type == JSONDataType::Number)
    return (float)m_nr;
  return 0.f;
}
double JSONEntry::toDouble() const {
  if ( this == nullptr )
    return 0.0;
  if ( m_type == JSONDataType::Number)
    return m_nr;
  return 0.0;
}


JSON *JSON::parseText( std::wstring &input ){
  JSONTokeniser in( input );
 
  JSONEntry *currentToken = &m_root;
  bool readIdentefier{true};
  while( currentToken && in.nextToken() ) {
    if ( in.tokenType() == TokenType::Symbol ) {
      switch ( in.symbol() ) {
        case '}':
        case ']': 
          currentToken = currentToken->m_parent; 
          break;
        case '{':
          currentToken->m_type = JSONDataType::Object;
          currentToken = new JSONEntry(currentToken);
          readIdentefier = true;
          break;
        case '[':
          currentToken->m_type = JSONDataType::Array;
          currentToken = new JSONEntry(currentToken);
          currentToken->m_name = L"0";
          readIdentefier = false;
          break;
        case ',':
          currentToken = new JSONEntry(currentToken->m_parent);
          if ( currentToken->m_parent->m_type == JSONDataType::Array ) {
            currentToken->m_name = currentToken->m_parent->arrayIndex();
          }
          else
            readIdentefier = true;
          break;
        case ':': //nur zur kontrolle, sonst anderst gelöst
          if ( readIdentefier )
            std::cout << "!!! Hier läuft was falsch !!!" << std::endl;
          break;   
      }
    }
    else if (in.tokenType() == TokenType::StringConst ) {
      if ( readIdentefier )
        currentToken->m_name = in.stringVal();
      else {
        currentToken->m_type = JSONDataType::Text;
        currentToken->m_text = in.stringVal();
      }
      readIdentefier = false;
    }
    else if (in.tokenType() == TokenType::NumberConst ) {      
      currentToken->m_type = JSONDataType::Number;
      currentToken->m_nr = in.numberVal();
    }
    else if (in.tokenType() == TokenType::BoolConst ) {      
      currentToken->m_type = JSONDataType::Bool;
      currentToken->m_bool = in.boolVal();
    }
  }
  return this;
} 

const JSONEntry *JSON::findElement( const JSONEntry *entry, const wchar_t *sub ) const {
  if ( entry->m_type == JSONDataType::Array || entry->m_type== JSONDataType::Object ) {
    for ( JSONEntry* e: entry->m_members ) {
      if ( e->m_name == sub )
        return e;
    }
  }
  return nullptr;
}
