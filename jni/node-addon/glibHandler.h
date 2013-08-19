#ifndef GLIBHANDLER_H
#define GLIBHANDLER_H

#include <node.h>
#include <string>
#include <list>


enum EVENT_TYPE{
  CONNECT_OUT=0,
  CONNECT_BACK,
  DISCONNECT_OUT,
  DISCONNECT_BACK,
  HANDLE_NOTIFY_BACK,
  HANDLE_INDICATOR_BACK,
  CHAR_WRITE_REQ_OUT,
  CHAR_WRITE_REQ_BACK,
  CHAR_WRITE_CMD_OUT,
  CHAR_READ_HND_OUT,
  CHAR_READ_HND_BACK,
  CHAR_READ_DESC_OUT,
  CHAR_READ_DESC_BACK
}; 

enum CONN_STATE{
  STATE_DISCONNECTED=0,
  STATE_CONNECTING,
  STATE_CONNECTED
};
  
struct messageQ {
  EVENT_TYPE event;
  char addr[20];
  int retval;
  int handle;
  int offset;
  int uuid;
  // we are making this a pointer. it may be a memory leak but i think they are freeing it somewhere inside gatt.c
  uint8_t *value;
  //unsigned long uldata;
  // ideally this would be dynamic...
  unsigned char handle_data[64];
  unsigned char handle_data_len;
  size_t plen;

};
  






class GlibHandler{
  public:
  static GlibHandler* Instance();
  static void AddEventToQ(struct messageQ *m, std::list<struct messageQ *> &Q,uv_mutex_t &qMutex);  
  static void AddEventToGLIBQ(struct messageQ *m);
  static void AddEventToJSQ(struct messageQ *m);
  static struct messageQ * RemoveEventFromGLIBQ(char *addr);
  static struct messageQ * RemoveEventFromJSQ(char *addr);  
  static struct messageQ * RemoveEventFromQ(char *addr,  std::list<struct messageQ *> &Q,uv_mutex_t &qMutex);  

  // coming in from JS
  static uv_mutex_t m_GLIBMutex;
  static std::list<struct messageQ *> m_GLIBQ;

  // going out to JS
  static uv_mutex_t m_JSMutex;
  static std::list<struct messageQ *> m_JSQ;



  // maintain who we are with
  static char m_currentAddress[20];
  static enum CONN_STATE m_connectionState;
  static void ChangeState(enum CONN_STATE newState,char *newAddr);
  static uv_mutex_t m_stateMutex;


  
  



  private:
  GlibHandler(){};  // Private so that it can  not be called
  GlibHandler(GlibHandler const&){};             // copy constructor is private
  //GlibHandler& operator=(GlibHandler const&){};  // assignment operator is private
  static GlibHandler* m_pInstance;

};


#endif
