#include "webSocketFrame.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>


WebSocketData::WebSocketData( WebSocketDataFrame *data ) 
{
  m_fin = data->fin();
  m_opcode = (OPCODE) data->opcode();
  m_payloadLng = data->payloadLng();
  
  setData( data->data(), data->mask(), data->maskKey() );
};
WebSocketData::WebSocketData( OPCODE op, const wchar_t* data, size_t lng ) 
{  
  lng *=2;
  m_opcode = op;

  m_data = new char[lng];
  m_payloadLng = wcstombs(m_data, data, lng);
};
WebSocketData::~WebSocketData() 
{
  if ( m_data ){
    delete[] m_data;
  }
};

uint WebSocketData::sendPackage( int socket ) 
{
  unsigned long long size = 0;
  auto data = makePackage( &size );
  uint count = send( socket, data, size, 0 );
  delete[] data;
  //m_data = nullptr;
  return count;
};

void WebSocketData::setData( const char* data, bool useMask, uint maskkey ) 
{
  if ( m_data ){    
    delete[] m_data;
  }
  if ( m_opcode == oc_text)
    m_data = new char[m_payloadLng+1]{0};
  else
    m_data = new char[m_payloadLng]{0};

  if ( useMask ) {
    char* mask = (char*)(&maskkey);
    for (unsigned long long i = 0; i < m_payloadLng; i++)
      m_data[i] =  data[i] ^ mask[i % 4];
  }
  else {
    memcpy( m_data, data, m_payloadLng );
  }
};
const char* WebSocketData::makePackage ( unsigned long long *size )
{
  char* package = new char[m_payloadLng+14]; // max size;
  WebSocketDataFrame *frame = reinterpret_cast<WebSocketDataFrame*>(package);
  frame->m_fin = 1;
  frame->m_res = 0;
  frame->m_opcode = (uint)m_opcode;
  frame->m_mask = 0;
  
  if ( m_payloadLng > 65535 ){
    frame->m_payloadLng = 127;
    std::cout << "make bigext Package" << std::endl;
    frame->m_bigext_PayloadLng = swapEndian(m_payloadLng);
    memcpy( frame->m_bigext_data, m_data, m_payloadLng);
  }
  else if ( m_payloadLng > 125){
    frame->m_payloadLng = 126;
    frame->m_ext_PayloadLng = swapEndian((short)m_payloadLng);
    memcpy( frame->m_ext_data, m_data, m_payloadLng);
  }
  else{
    frame->m_payloadLng = m_payloadLng;
    if ( m_payloadLng )
      memcpy( frame->m_data, m_data, m_payloadLng);
  }
  if ( size )
    *size = frame->framesize();
  return package;
}; 