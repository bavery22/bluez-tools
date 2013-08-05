#include <node.h>
#include "device.h"
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


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


using namespace v8;

// a list of devices that we will add to as we find them
struct info{
  char buf1[256];
  char buf2[256];
  int handle;
  uint8_t value;
  size_t plen;
  v8::Persistent<v8::Function> callback;

};


static uv_mutex_t conn_mutex;
static GMainLoop *event_loop;
static uint16_t conn_handle = 0;
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
  fprintf(stderr,"bavery: connect_cb\n");

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


  g_main_loop_quit(event_loop);
  uv_mutex_unlock(&conn_mutex);
  
}



static void do_char_write_work(uv_work_t *req)
{
  struct info *i = (struct info *) req->data;

  fprintf(stderr,"do_char_write_work\n");
  fprintf(stderr,"do_char_write_work : handle->0x%x  value->0x%x plen=%d\n",i->handle,i->value,i->plen);
  //event_loop = g_main_loop_new(NULL, FALSE);

  gatt_write_char(attrib, i->handle, &(i->value), i->plen, NULL, NULL);

  //g_main_loop_run(event_loop);
  //g_main_loop_unref(event_loop);

  
  fprintf(stderr,"do_char_write_work exiting\n");


}
static void done_char_write_work(uv_work_t *req, int status)
{
  fprintf(stderr,"done_char_write_work   status = %d\n");
#if 0
  struct devices *d = (struct devices *) req->data;
  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("DONE DID  WORK")) };
  d->callback->Call(Context::GetCurrent()->Global(), argc, argv);
  for (int i=0;i<d->numEntries;i++)
    {
      Local<Value> argv[argc] = { Local<Value>::New(String::New(d->gEntries[i])) };
      d->callback->Call(Context::GetCurrent()->Global(), argc, argv);
      free(d->gEntries[i]);
    }
#endif
}



static void do_connect_work(uv_work_t *req)
{
  struct info *i = (struct info *) req->data;

  fprintf(stderr,"do_connect_work\n");
  uv_mutex_init(&conn_mutex);
  uv_mutex_lock(&conn_mutex);
  
  //event_loop = g_main_loop_new(NULL, FALSE);


  fprintf(stderr,"bavery calling gatt_connect to addr: %s\n",i->buf1);
  iochannel =  gatt_connect("hci0", i->buf1, "public", "low",
	       NULL, NULL, connect_cb);

  if (iochannel == NULL)
    fprintf(stderr,"got a null iochannel from gatt_connect\n");

  //g_main_loop_run(event_loop);
  // this will wait until the conn_cb unlocks it. then we can exit and return.
  uv_mutex_lock(&conn_mutex);

  //g_main_loop_unref(event_loop);
  fprintf(stderr,"do_connect_work exiting\n");


}
static void done_connect_work(uv_work_t *req, int status)
{
  fprintf(stderr,"done_connect_work   status = %d\n");
#if 0
  struct devices *d = (struct devices *) req->data;
  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("DONE DID  WORK")) };
  d->callback->Call(Context::GetCurrent()->Global(), argc, argv);
  for (int i=0;i<d->numEntries;i++)
    {
      Local<Value> argv[argc] = { Local<Value>::New(String::New(d->gEntries[i])) };
      d->callback->Call(Context::GetCurrent()->Global(), argc, argv);
      free(d->gEntries[i]);
    }
#endif
}


static void do_gloop_work(uv_work_t *req)
{

  fprintf(stderr,"do_gloop_work\n");

  
  event_loop = g_main_loop_new(NULL, FALSE);

  while (1){
    g_main_loop_run(event_loop);
  fprintf(stderr,"do_gloop_work musta gotten a quit somewhere \n");
  }
  


}
static void done_gloop_work(uv_work_t *req, int status)
{
  fprintf(stderr,"done_gloop_work   status = %d   AAAARRRRGH\n");

}



Device::Device() {
  if (!m_glibhandler)
    m_glibhandler = GlibHandler::Instance();

};
Device::~Device() {};

void Device::Init(Handle<Object> target) {

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Device"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
      FunctionTemplate::New(PlusOne)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("CWR"),
      FunctionTemplate::New(CWR)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("Connect"),
      FunctionTemplate::New(Connect)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("CharWriteCommand"),
      FunctionTemplate::New(CharWriteCommand)->GetFunction());

  Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Device"), constructor);

}

Handle<Value> Device::New(const Arguments& args) {
  HandleScope scope;

  Device* obj = new Device();
  //obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->counter_ =0;
  //Local<Function> cb = Local<Function>::Cast(args[0]);
  obj->m_cb = v8::Persistent<v8::Function>::New(args[0]);
  obj->m_state = STATE_DISCONNECTED;
  /*
const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("hello world")) };
  obj->CB->Call(Context::GetCurrent()->Global(), argc, argv);
  */
  uv_work_t *req = new uv_work_t;
  uv_queue_work(uv_default_loop(),req,do_gloop_work,done_gloop_work);


  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> Device::PlusOne(const Arguments& args) {
  HandleScope scope;

  Device* obj = ObjectWrap::Unwrap<Device>(args.This());
  obj->counter_ += 1;
  // call the callback
  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("cows")) };
  obj->m_cb->Call(Context::GetCurrent()->Global(), argc, argv);
  
  return scope.Close(Number::New(obj->counter_));
}

//char-write-req
Handle<Value> Device::CWR(const Arguments& args) {
  HandleScope scope;

  Device* obj = ObjectWrap::Unwrap<Device>(args.This());
  //  my_devices->timeToScan  = args[0]->IsUndefined() ? 5 : args[0]->NumberValue();

  return scope.Close(Undefined());
  
}

//connect method
Handle<Value> Device::Connect(const Arguments& args) {
  HandleScope scope;
  Device* obj = ObjectWrap::Unwrap<Device>(args.This());
  // get the param
  v8::String::Utf8Value param1(args[0]->ToString());
  std::string text1 = std::string(*param1);
  const char * addr = text1.c_str();
  uv_work_t *req = new uv_work_t;
  struct info *my_info = new struct info;

  strcpy(my_info->buf1,addr);
  req->data = my_info;

  fprintf(stderr,"Coonect to %s\n",addr);
  uv_queue_work(uv_default_loop(),req,do_connect_work,done_connect_work);


#if 0
  obj->m_state = STATE_CONNECTING;
  iochannel =  gatt_connect("hci0", addr, "public", "low",
	       NULL, NULL, connect_cb);

  if (iochannel == NULL)
    fprintf(stderr,"got a null iochannel from gatt_connect\n");
#endif

  return scope.Close(Undefined());
  
}


//char_write_command method
// no cb functions, so no libuv shenanigans
Handle<Value> Device::CharWriteCommand(const Arguments& args) {
  HandleScope scope;
  Device* obj = ObjectWrap::Unwrap<Device>(args.This());
  // get the param
  v8::String::Utf8Value param1(args[0]->ToString());
  std::string text1 = std::string(*param1);
  const char * handleString = text1.c_str();
  // get the value
  v8::String::Utf8Value param2(args[1]->ToString());
  std::string text2 = std::string(*param2);
  const char * valueString = text2.c_str();
  uint8_t *value;

  uv_work_t *req = new uv_work_t;
  struct info *my_info = new struct info;


  //my_info->handle = strtohandle(handleString);
  //my_info->handle = strtohandle("0x0039");
  my_info->handle = 0x0039;
  //my_info->plen=gatt_attr_data_from_string(valueString, &value);
  //my_info->plen=gatt_attr_data_from_string("20", &value);
  my_info->plen=1;
  // noop here but needed for the char_write_req version l8r
  //my_info->value = *value;
  my_info->value = 0x20;
  req->data = my_info;

  fprintf(stderr,"CharWriteCommand handle= %s/0x%x value=%s/%0x%x\n",handleString,my_info->handle,valueString,my_info->value);

  uv_queue_work(uv_default_loop(),req,do_char_write_work,done_char_write_work);



  return scope.Close(Undefined());
  
}
