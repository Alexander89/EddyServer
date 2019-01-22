#ifndef WEBSOCKETFRAME_H
#define WEBSOCKETFRAME_H
#include <string>
#include <iostream>
#include "helperfunctions.h"

template< class T >
T toMSB( T value, int bits ) {
  T res{0};
  for ( int i = 0; i < bits; ++i )
    res |= (((value>>i)&1)<<(bits-1-i));
  return res;
}

#pragma pack(push, 1)
/*
  struct WebSocketData_S_D {
    unsigned int m_opcode : 4;
    unsigned int m_res : 3;
    unsigned int m_fin : 1;
    unsigned int m_payloadLng : 7;
    unsigned int m_mask : 1;
    char m_data[0];  
  };
  struct WebSocketData_S_MD {
    unsigned int m_opcode : 4;
    unsigned int m_res : 3;
    unsigned int m_fin : 1;
    unsigned int m_payloadLng : 7;
    unsigned int m_mask : 1;
    unsigned int m_mask_key : 32;
    char m_data[0];  
  };

  struct WebSocketData_M_D {  
    unsigned int m_opcode : 4;
    unsigned int m_res : 3;
    unsigned int m_fin : 1;
    unsigned int m_payloadLng : 7;
    unsigned int m_mask : 1;
    unsigned short m_ext_PayloadLng : 16;
    char m_mask_data[0];
  };
  struct WebSocketData_M_MD {
    unsigned int m_opcode : 4;
    unsigned int m_res : 3;
    unsigned int m_fin : 1;
    unsigned int m_payloadLng : 7;
    unsigned int m_mask : 1;
    unsigned int m_key : 32;
    unsigned short m_ext_PayloadLng : 16;
    unsigned int m_mask_Key : 32;
    char m_mask_data[0];
  };

  struct WebSocketData_L_D {
    unsigned int m_opcode : 4;
    unsigned int m_res : 3;
    unsigned int m_fin : 1;
    unsigned int m_payloadLng : 7;
    unsigned int m_mask : 1;
    unsigned int m_key : 32;
    unsigned long long m_ext_PayloadLng : 64;  
    char m_ext_mask_data[0];
  };
  struct WebSocketData_L_MD {
    unsigned int m_opcode : 4;
    unsigned int m_res : 3;
    unsigned int m_fin : 1;
    unsigned int m_payloadLng : 7;
    unsigned int m_mask : 1;
    unsigned int m_key : 32; 
    unsigned long long m_ext_PayloadLng : 64; 
    unsigned int m_ext_mask_Key : 32;
    char m_ext_mask_data[0];
  };
*/

struct WebSocketDataFrame {
  unsigned int m_opcode : 4;
  unsigned int m_res : 3;
  unsigned int m_fin : 1;
  unsigned int m_payloadLng : 7;
  unsigned int m_mask : 1;
  union {
    struct {
      union {
        char m_data[0];
        struct {
          unsigned int m_mask_Key : 32;
          char m_mask_data[0];
        };
      };
    };
    struct {
      unsigned short m_ext_PayloadLng : 16;
      union {
        char m_ext_data[0];
        struct {
          unsigned int m_ext_mask_Key : 32;
          char m_ext_mask_data[0];
        };
      };
    };    
    struct {
      unsigned long long m_bigext_PayloadLng : 64;
      union {
        char m_bigext_data[0];
        struct {
          unsigned int m_bigext_mask_Key : 32;
          char m_bigext_mask_data[0];
        };
      };
    };
  };

  //helper functions
  bool fin() const { return m_fin; }
  int opcode() const { return  m_opcode; }
  bool mask() const { return m_mask; }

  unsigned long long payloadLng() const {
    const uint lng = m_payloadLng;
    if (lng == 126)
      return swapEndian(m_ext_PayloadLng);
    else if (lng == 127)
      return swapEndian(m_bigext_PayloadLng);
    return lng;
  };
  unsigned int maskKey() const {
    if (m_payloadLng == 126)
      return m_ext_mask_Key;
    else if (m_payloadLng == 127)
      return m_bigext_mask_Key;
    else 
      return m_mask_Key;
  };
  const char* data() const {
    const uint lng = m_payloadLng;
    if (lng == 126){
      if (m_mask)
        return m_ext_mask_data;
      else
        return m_ext_data;
    }
    else if (lng == 127){
      if (m_mask)
        return m_bigext_mask_data;
      else
        return m_bigext_data;
    }
    else {
      if (m_mask)
        return m_mask_data;
      else
        return m_data;
    }
  };
  int minLng() const {
    if (m_payloadLng == 126)
      return m_mask ? 8 : 4;
    else if (m_payloadLng == 127)
      return m_mask ? 14 : 10;
    else 
      return m_mask ? 6 : 2;    
  } ;
  unsigned int framesize() const { return minLng() + payloadLng(); }
};
#pragma pack(pop)


enum OPCODE {
  oc_continuation = 0, //%x0 denotes a continuation frame
  oc_text = 1,         //%x1 denotes a text frame
  oc_binary = 2,       //%x2 denotes a binary frame
                    //%x3-7 are reserved for further non-control frames
  oc_close = 8,        //%x8 denotes a connection close
  oc_ping = 9,         //%x9 denotes a ping
  oc_pong = 10,        //%xA denotes a pong
                    //%xB-F are reserved for further control frames
};

class WebSocketData  {
public:
  explicit WebSocketData( WebSocketDataFrame *data ) ;
  WebSocketData( OPCODE op, const wchar_t* data, size_t lng ) ;
  ~WebSocketData() ;
  bool fin() { return m_fin; }
  OPCODE opcode() { return m_opcode; }
  unsigned long long dataLng() { return m_payloadLng; }
  char *data() { return m_data; }

  uint sendPackage( int socket );
private:
  void setData( const char* data, bool useMask, uint maskkey );
 const char* makePackage ( unsigned long long *size = nullptr);
private:
  bool m_fin{false};
  OPCODE m_opcode{oc_close};
  unsigned long long m_payloadLng{0};
  char* m_data{0};
};

#endif