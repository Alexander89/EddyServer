#include "webServer.h"
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

struct WebSocketDataFrame {
  bool m_fin : 1;
  bool m_res1 : 1;
  bool m_res2 : 1;
  bool m_res3 : 1;
  unsigned int m_opcode : 4;
  bool m_mask : 1;
  unsigned int m_payloadLng : 7;
  union {
    struct {
      union {
        const char* m_data;
        struct {
          unsigned int m_mask_Key : 32;
          const char* m_mask_data;
        };
      };
    };
    struct {
      unsigned int m_ext_PayloadLng : 16;
      union {
        const char* m_ext_data;
        struct {
          unsigned int m_ext_mask_Key : 32;
          const char* m_ext_mask_data;
        };
      };
    };    
    struct {
      unsigned long long m_bigext_PayloadLng : 64;
      union {
        const char* m_bigext_data;
        struct {
          unsigned int m_bigext_mask_Key : 32;
          const char* m_bigext_mask_data;
        };
      };
    };
  };

  //helper functions
  bool fin() const { return m_fin; }
  int opcode() const { return m_opcode; }
  bool mask() const { return m_mask; }

  unsigned long long payloadLng() const {
    if (m_payloadLng == 126)
      return m_ext_PayloadLng;
    else if (m_payloadLng == 127)
      return m_bigext_PayloadLng;
    else 
      return m_payloadLng;
  }
  unsigned int maskKey() const {
    if (m_payloadLng == 126)
      return m_ext_mask_Key;
    else if (m_payloadLng == 127)
      return m_bigext_mask_Key;
    else 
      return m_mask_Key;
  }
  const char* data() const {
    if (m_payloadLng == 126){
      if (m_mask)
        return m_ext_mask_data;
      else
        return m_ext_data;
    }
    else if (m_payloadLng == 127){
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
  }
};

WebServer::WebServer( unsigned short port ) : Server(port)
{

}

void WebServer::handleHttp( const char* data, int lng ) {
  string request(data, lng);
  vector<string> v = splitString(request, "\r\n");

  auto getReq = splitString( v.front(), " " );

  for (auto i: v) {
    cout << "line " << i << endl;
  }
}

void WebServer::handleData( const char* data, int lng ) {

}

void WebServer::receveData( const char* data, int lng ) {
  cout << "receve Data: " << lng << endl;
  if ( lng >= 17 ) { // minimal size for httpRequests. is bigger on real requests
    if ( !memcmp( data, "GET", 3 ) ) {
      handleHttp(data, lng);
      return;
    }
  }
  
  handleData(data, lng);  
}


vector<string> WebServer::splitString( string req, const char *seperator, vector<string> vec ){
  size_t pos = req.find(seperator); 
  if ( pos == string::npos )
    return vec;
  if ( pos > 0 )
    vec.push_back( req.substr(0, pos) );
  return splitString( req.substr( pos + strlen(seperator) ), seperator, vec );
}