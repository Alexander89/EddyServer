#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H
#include <iostream>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include "SHA1.h"

inline std::vector<std::string> *splitString ( std::string req, const char *seperator, std::vector<std::string> *vec = new std::vector<std::string>() ){
  size_t pos = req.find(seperator); 
  if ( pos == std::string::npos ){
    vec->push_back( req );
    return vec;
  }
  if ( pos > 0 )
    vec->push_back( req.substr(0, pos) );
  return splitString( req.substr( pos + strlen(seperator) ), seperator, vec );
}
inline std::vector<std::wstring> *splitWString ( std::wstring req, const wchar_t *seperator, std::vector<std::wstring> *vec = new std::vector<std::wstring>() ){
  size_t pos = req.find(seperator); 
  if ( pos == std::wstring::npos ){
    vec->push_back( req );
    return vec;
  }
  if ( pos > 0 )
    vec->push_back( req.substr(0, pos) );
  return splitWString( req.substr( pos + wcslen(seperator) ), seperator, vec );
}

inline void printBits ( const char *data, uint len) {
  for (uint i = 0; i < len; ++i) {
    //for (int j = 7; j >= 0; --j) {
    for (int j = 0; j < 8; ++j) {      
      unsigned char byte = data[i];
      std::cout <<( ((byte >> j) & 1) ? 1 : 0);
    }
    std::cout << " ";
  }
  std::cout << std::endl;
}
template< typename T >
T hypot3(T x, T y, T z) {
  return sqrt(x*x + y*y + z*z);
}

template< typename T >
T swapEndian ( T val ){
  uint size = sizeof(T);
  if ( size == 2 )  
    return (val << 8) | ((val >> 8) & 0x00ff); // right-shift sign-extends, so force to zero
  else if ( size == 4 ) {
    return (val << 24) |
        ((val <<  8) & 0x00ff0000) |
        ((val >>  8) & 0x0000ff00) |
        ((val >> 24) & 0x000000ff);
  }
  else if ( size == 8 ) {
    return (val << 56) |
        ((val << 40) & 0x00ff000000000000) |
        ((val << 24) & 0x0000ff0000000000) |
        ((val <<  8) & 0x000000ff00000000) |
        ((val >>  8) & 0x00000000ff000000) |
        ((val >> 24) & 0x0000000000ff0000) |
        ((val >> 40) & 0x000000000000ff00) |
        ((val >> 56) & 0x00000000000000ff);
  }
  return val;
}
/*
inline short swapEndian ( short in ){
  short res{0};  
  for (int shift = 0; shift < 2; ++shift)   
    res |= ((in >> (shift*8)) & 1)<<((1-shift)*8);
  return res;
}
inline int swapEndian ( int in ){
  int res{0};  
  for (int shift = 0; shift < 4; ++shift)   
    res |= ((in >> (shift*8)) & 1)<<((3-shift)*8);
  return res;
}
inline uint swapEndian ( uint in ) { return (uint)swapEndian((int)in); }
inline long long swapEndian ( long long in ){
  long long res{0};  
  for (int shift = 0; shift < 8; ++shift)   
    res |= ((in >> (shift*8)) & 1)<<((7-shift)*8);
  return res;
}
inline unsigned long long swapEndian ( unsigned long long in ) { return (unsigned long long)swapEndian((long long)in); }
*/
inline char *swapBits (const char *in, uint len){
  char *res = new char[len];
  for (uint i = 0; i < len; ++i)
  {
    char temp{0};
    for (uint shift = 0; shift < 8; ++shift)    
      temp |= ((in[i] >> shift) & 1)<<(7-shift);
    res[i] = temp;
  }
  return res;
}

const char MagicCode[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const char Base64CData[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct Base64DecodingStruct {
  union {
    struct {
      unsigned char c3;
      unsigned char c2;
      unsigned char c1;
    };
    struct {
      unsigned int partern4:6;
      unsigned int partern3:6;
      unsigned int partern2:6;
      unsigned int partern1:6;
    };
  };
};

inline const char *toBase64(const unsigned char *in, uint len, uint *retLng = nullptr){
  int resLen = (len / 3 * 4 )+ 4;
  char *res = new char[resLen]{0};
  if ( retLng )
    *retLng = resLen;
  Base64DecodingStruct decodeMask;
  
  for (uint i = 0; i <= len/3; i++) {
    const unsigned char* c = in + (i*3);
    decodeMask.c1 = *c;    
    decodeMask.c2 = (i*3+1)<len ? *(c+1) : 0;
    decodeMask.c3 = (i*3+2)<len ? *(c+2) : 0;
    res[ i*4 ] =   Base64CData[decodeMask.partern1];
    res[ i*4+1 ] = Base64CData[decodeMask.partern2]; 
    res[ i*4+2 ] = Base64CData[decodeMask.partern3];
    res[ i*4+3 ] = Base64CData[decodeMask.partern4];
  }
  switch(len%3){
    case 1: res[resLen-2] = '=';
    case 2: res[resLen-1] = '=';
  }
  res[resLen] = 0;

  return res; 
}

inline const char *createAcceptKey( const char *in, uint len) {
  unsigned char input[70];
  memcpy(input, in, len);
  memcpy(input+len, MagicCode, 36);

  SHA1 hash;
  hash.update(input, 36+len);
  hash.final();
  unsigned char res[21]{0};
  hash.getHash( res );
  return toBase64(res, 20);
}

#endif //HELPERFUNCTIONS_H