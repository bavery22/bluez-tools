#ifndef GLIBHANDLER_H
#define GLIBHANDLER_H

#include <node.h>
#include <string>
#include <vector>

enum EVENT_TYPE{
  CONNECT_OUT,
  CONNECT_BACK,
  DISCONNECT_OUT,
  DISCONNECT_BACK,
  INDICATOR_OUT,
  INDICATOR_BACK,
  CHAR_WRITE_REQ_OUT,
  CHAR_WRITE_REQ_BACK,
  CHAR_WRITE_CMD_OUT
}; 
  
struct messageQ {
  EVENT_TYPE event;
  char addr[256];
  int handle;
  uint8_t value;
  size_t plen;

};
  






class GlibHandler{
  public:
  static GlibHandler* Instance();


  private:
  GlibHandler(){};  // Private so that it can  not be called
  GlibHandler(GlibHandler const&){};             // copy constructor is private
  //GlibHandler& operator=(GlibHandler const&){};  // assignment operator is private
  static GlibHandler* m_pInstance;
  
  
  uv_mutex_t m_mutex;
  std::vector<struct messageQ *> m_Q;
};






#endif
