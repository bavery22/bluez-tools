#include <stddef.h>  // defines NULL
#include "glibHandler.h"



enum {
  BT_UUID_UNSPEC = 0,
  BT_UUID16 = 16,
  BT_UUID32 = 32,
  BT_UUID128 = 128,
} ;


extern "C" {

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <glib.h>
#include "att.h"
#include "btio.h"
#include "gattrib.h"
#include "gatt.h"
#include "gatttool.h"

}

// Static class initializers
uv_mutex_t GlibHandler::m_GLIBMutex;
uv_mutex_t GlibHandler::m_JSMutex;
std::list<struct messageQ *> GlibHandler::m_GLIBQ;
std::list<struct messageQ *> GlibHandler::m_JSQ;
char GlibHandler::m_currentAddress[20];
enum CONN_STATE GlibHandler::m_connectionState;
uv_mutex_t GlibHandler::m_stateMutex;



// these should not be statics ...
static uint16_t conn_handle = 0;
static GMainLoop *event_loop;
static GAttrib *attrib = NULL;
static GIOChannel *iochannel = NULL;



static int strtohandle(const char *src)
{
    char *e;
    int dst;

    //errno = 0;
    dst = strtoll(src, &e, 16);
    //if (errno != 0 || *e != '\0')
    //    return -EINVAL;

    return dst;
}

static void disconnect_io()
{
  if (GlibHandler::m_connectionState== STATE_DISCONNECTED)
    return;

  g_attrib_unref(attrib);
  attrib = NULL;


  g_io_channel_shutdown(iochannel, FALSE, NULL);
  g_io_channel_unref(iochannel);
  iochannel = NULL;

  fprintf(stderr,"\ndisconnect_io conn_handle = %d\n",conn_handle);
  
  struct messageQ *m2 = malloc (sizeof(struct messageQ ));
  m2->event=DISCONNECT_BACK;
  strcpy(m2->addr ,GlibHandler::m_currentAddress);
  GlibHandler::AddEventToGLIBQ(m2);

}


static gboolean channel_watcher(GIOChannel *chan, GIOCondition cond,
                gpointer user_data)
{
  fprintf(stderr,"\nDISCONNECTED(%04x)\n", conn_handle);
  disconnect_io();
  
  return FALSE;
}



static void events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
{
    uint8_t *opdu;
    uint16_t handle, i, olen;
    size_t plen;

    handle = att_get_u16(&pdu[1]);

    printf("\n");
    switch (pdu[0]) {
    case ATT_OP_HANDLE_NOTIFY:
        printf("NOTIFICATION(%04x): %04x ", conn_handle , handle);
        break;
    case ATT_OP_HANDLE_IND:
        printf("INDICATION(%04x): %04x ", conn_handle, handle);
        break;
    default:
        printf("ERROR(%04x): (16,256) Invalid opcode\n", conn_handle);
        return;
    }

    for (i = 3; i < len; i++)
        printf("%02x ", pdu[i]);


    if (pdu[0] == ATT_OP_HANDLE_NOTIFY)
        return;

    opdu = g_attrib_get_buffer(attrib, &plen);
    olen = enc_confirmation(opdu, plen);

    if (olen > 0)
        g_attrib_send(attrib, 0, opdu[0], opdu, olen, NULL, NULL, NULL);
}




static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
  fprintf(stderr,"bavery: connect_cb err=%d\n",err);
  
  if (err){
    struct messageQ *m2 = malloc (sizeof(struct messageQ ));
    m2->event=DISCONNECT_OUT;
    strcpy(m2->addr ,GlibHandler::m_currentAddress);
    GlibHandler::AddEventToGLIBQ(m2);

    return;
  }
  attrib = g_attrib_new(iochannel);
  g_attrib_register(attrib, ATT_OP_HANDLE_NOTIFY, events_handler,
		    attrib, NULL);
  g_attrib_register(attrib, ATT_OP_HANDLE_IND, events_handler,
		    attrib, NULL);
    
  GError *gerr = NULL;

  // get connection handle
  bt_io_get(iochannel, &gerr, BT_IO_OPT_HANDLE, &conn_handle,
	    BT_IO_OPT_INVALID);
  
  fprintf(stderr,"\nCONNECTED(%04x):  0\n", conn_handle);
  

  // after we put the addr in the userdata, add the in connect to the Q
  struct messageQ *m = malloc (sizeof(struct messageQ ));
  m->event=CONNECT_BACK;
  strcpy(m->addr ,GlibHandler::m_currentAddress);
  GlibHandler::AddEventToGLIBQ(m);

}

static   void HandleQEvent(struct messageQ * &m)
{
  fprintf(stderr,"HandleQEvent: current state = %d, current conn_addr = %s working on event %d  addr=%s\n",GlibHandler::m_connectionState,GlibHandler::m_currentAddress,
	  m->event,m->addr);
  // if the event cant be handled right now we push it to the back of the Q
  int handled = 0;
  switch (m->event){
  case CONNECT_OUT:
    switch (GlibHandler::m_connectionState){
    case STATE_DISCONNECTED:
      handled=1;
      fprintf(stderr,"bavery calling gatt_connect to addr: %s\n",m->addr);
      
      GlibHandler::ChangeState(STATE_CONNECTING,m->addr);
      iochannel =  gatt_connect("hci0", m->addr, "public", "low",
				NULL, NULL, connect_cb);

      if (iochannel == NULL)
        GlibHandler::ChangeState(STATE_DISCONNECTED,"");
      else
        g_io_add_watch(iochannel, G_IO_HUP, channel_watcher, NULL);

      break;
    case STATE_CONNECTING:
      fprintf(stderr,"Currently connecting, hold your horses\n");
      break;
    case STATE_CONNECTED:
      if (! strcasecmp(m->addr,GlibHandler::m_currentAddress))
	{
	  fprintf(stderr,"already connected to that addr\n");
	  handled=1;
	}
      else{
	fprintf(stderr,"already connected to %s, need to disconnect first\n",GlibHandler::m_currentAddress);
	// we want to connect to someone else so force the disconnect first.
	{
	  struct messageQ *m2 = malloc (sizeof(struct messageQ ));
	  m2->event=DISCONNECT_OUT;
	  strcpy(m2->addr ,GlibHandler::m_currentAddress);
	  GlibHandler::AddEventToGLIBQ(m2);
	}
      }
      break;
    }
    break;

  case CONNECT_BACK:
    //back events are handled by definition
    handled=1;
    switch (GlibHandler::m_connectionState){
    case STATE_DISCONNECTED:
      fprintf(stderr,"ERROR in DISCONNECTED BUT GOT A CONNECT)_BACH for addr %s\n",m->addr);
      break;
    case STATE_CONNECTING:
      GlibHandler::ChangeState(STATE_CONNECTED,m->addr);
      {
	struct messageQ *m2 = malloc (sizeof(struct messageQ ));
	m2->event=CONNECT_BACK;
	strcpy(m2->addr ,GlibHandler::m_currentAddress);
	GlibHandler::AddEventToJSQ(m2);
      }
      break;
    case STATE_CONNECTED:
      fprintf(stderr,"ERROR got a connect back while already connected addr=%s currentAddr=%s\n",m->addr,GlibHandler::m_currentAddress);
      break;
    }
    break;
  case DISCONNECT_OUT:
    handled=1;
    switch (GlibHandler::m_connectionState){
    case STATE_DISCONNECTED:
      break;
    case STATE_CONNECTING:
    case STATE_CONNECTED:
          disconnect_io();
      break;
    }
    break;
  case DISCONNECT_BACK:
    //back events are handled by definition
    handled=1;
    switch (GlibHandler::m_connectionState){
    case STATE_DISCONNECTED:
      fprintf(stderr,"already disconnected but got another disconnect from  addr: %s  ERROR \n",m->addr);
      break;
    case STATE_CONNECTING:
    case STATE_CONNECTED:
      {
	struct messageQ *m2 = malloc (sizeof(struct messageQ ));
	m2->event=DISCONNECT_BACK;
	strcpy(m2->addr ,GlibHandler::m_currentAddress);
	GlibHandler::AddEventToJSQ(m2);
      }
      GlibHandler::ChangeState(STATE_DISCONNECTED,"");
    }
    break;
  case CHAR_WRITE_CMD_OUT:
    switch (GlibHandler::m_connectionState){
      fprintf(stderr,"bavery-> CHAR_WRITE_CMD_OUT state = %d\n",GlibHandler::m_connectionState);
    case STATE_DISCONNECTED:
      // we got disconnected. reconnect and then run the command
      	{
	  struct messageQ *m2 = malloc (sizeof(struct messageQ ));
	  m2->event=CONNECT_OUT;
	  strcpy(m2->addr ,m->addr);
	  GlibHandler::AddEventToGLIBQ(m2);
	}
      break;
    case STATE_CONNECTING:
      // still connecting. not handled. nothing to do but wait.
      break;
    case STATE_CONNECTED:
      handled=1;
      fprintf(stderr,"bavery calling attrib = %d gatt_write_char handle=%d value=%d plen=%d\n",attrib,m->handle, *(m->value), m->plen);
      gatt_write_char(attrib, m->handle, m->value, m->plen, NULL, NULL);
      break;
    }
    break;

  default:
    fprintf(stderr,"unknown event in HandleQEvent: %d\n",m->event);
    handled=1; // delete the baddy
  }
  
  // if we handled the event, free it. otherwise put it back at the end of the Q
  fprintf(stderr,"HandleQEvent DONE handled = %d\n",handled);
  if (handled)
    free(m);
  else
    GlibHandler::AddEventToGLIBQ(m);
    
}


static void do_gloop_work(uv_work_t *req)
{


  fprintf(stderr,"do_gloop_work\n");
  event_loop = g_main_loop_new(NULL, FALSE);
  GlibHandler* glH = GlibHandler::Instance();

  while (1){
    if (g_main_context_iteration(NULL,FALSE))
      {
	fprintf(stderr,"Found a glib event to handle\n");
      }
    // note we should be checking our state here
    
    // here should check our eventQ to see if we need to do something (like send out a connect)
    struct messageQ *m=glH->RemoveEventFromGLIBQ(glH->m_currentAddress);
    if (m)
      HandleQEvent(m);
    

    // thread yield would be better here but not in uv by default. may add l8r
    usleep(500);
  }
}


static void done_gloop_work(uv_work_t *req, int status)
{
  fprintf(stderr,"done_gloop_work   SHOULD NEVER EVER GET HERE status = %d   AAAARRRRGH\n");

}





// Global static pointer used to ensure a single instance of the class.
GlibHandler* GlibHandler::m_pInstance = NULL;  

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/
  
GlibHandler* GlibHandler::Instance()
{
  if (!m_pInstance) {  // Only allow one instance of class to be generated.
    m_pInstance = new GlibHandler;

    // start the glib work thread
    uv_work_t *req = new uv_work_t;
    uv_queue_work(uv_default_loop(),req,do_gloop_work,done_gloop_work);

    //set up the mutex
    uv_mutex_init(&m_GLIBMutex);
    uv_mutex_init(&m_JSMutex);
    uv_mutex_init(&m_stateMutex);    

    // we start disconnected of course
    GlibHandler::ChangeState(STATE_DISCONNECTED,"");
    
  }
  return m_pInstance;
}


struct messageQ * GlibHandler::RemoveEventFromQ(char *addr,  std::list<struct messageQ *> &Q,uv_mutex_t &qMutex)
{
  
    struct messageQ *m=NULL;
    if (Q.size()>0)
      fprintf(stderr,"RemoveEventFromQ size=%d addr = %s strlen(addr)=%d\n",Q.size(),addr,strlen(addr));
    uv_mutex_lock(&qMutex);
    
    // if we have an addr passed in, then return the first of that addr.
    // if we get an addr of "" passed in then just pass back the first event
    if (strlen(addr)){
      for (std::list<struct messageQ *>::iterator it=Q.begin(); it != Q.end(); ++it)
	if (!strcasecmp(addr,(*it)->addr)){
	  m=(*it);
	  Q.erase(it);
	  break;
	}
    }
    else{
      //fprintf(stderr,"no addr Q size=%d\n",Q.size());
      if (Q.size()){
	m = Q.front();
	Q.pop_front();
      }

      if (m)
	fprintf(stderr,"no addr Q post pop size=%d and m type = %d and addr = %s\n",Q.size(),m->event,m->addr);
    }
    uv_mutex_unlock(&qMutex);
    return m;
}
struct messageQ * GlibHandler::RemoveEventFromGLIBQ(char *addr)
{
  return RemoveEventFromQ(addr,m_GLIBQ,m_GLIBMutex);

}
struct messageQ * GlibHandler::RemoveEventFromJSQ(char *addr)
{

  return RemoveEventFromQ(addr,m_JSQ,m_JSMutex);
}


void GlibHandler::AddEventToQ(struct messageQ *data,std::list<struct messageQ *> &Q,uv_mutex_t &qMutex)
{
    uv_mutex_lock(&qMutex);
    Q.push_back(data);
    uv_mutex_unlock(&qMutex);


}

void GlibHandler::AddEventToGLIBQ(struct messageQ *data)
{
  AddEventToQ(data,m_GLIBQ,m_GLIBMutex);
}
void GlibHandler::AddEventToJSQ(struct messageQ *data)
{
  AddEventToQ(data,m_JSQ,m_JSMutex);
}



void GlibHandler::ChangeState(enum CONN_STATE newState,char *newAddr)
{
  uv_mutex_lock(&m_stateMutex);
  m_connectionState = newState;
  // not all state changes change the address
  if (newAddr)
    strcpy(m_currentAddress,newAddr);
    uv_mutex_unlock(&m_stateMutex);

}
