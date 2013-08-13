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


// Static class initializers
static GlibHandler* Device::m_glibhandler;


using namespace v8;

// a list of devices that we will add to as we find them
struct info{
  char addr[20];
  struct messageQ *m;
  v8::Persistent<v8::Function> callback;
};

static void do_queue_work(uv_work_t *req)
{
  struct info  *my_info= req->data;
  fprintf(stderr,"do_queue_work\n");
  while (1){
    // here should check our eventQ to see if we need to do something (like send out a connect)
    struct messageQ *m=    GlibHandler::RemoveEventFromJSQ(my_info->addr);
    if (m){
      fprintf(stderr,"found an event for address:%s going to done next\n",my_info->addr);
      my_info->m = m;
      return;
    }
    // thread yield would be better here but not in uv by default. may add l8r
    usleep(500);
  }
}


static void done_queue_work(uv_work_t *req, int status)
{
  struct info  *my_info= req->data;
  


  if (! my_info->m){
    fprintf(stderr,"no event for addr %s!!!!!\n",my_info->addr);
  }
  else
    {

      fprintf(stderr,"event = %d for addr %s\n",my_info->m->event,my_info->addr);
  
      const unsigned argc = 1;
      //Local<Value> argv[argc] = { Local<Value>::New((int) my_info->m->event) };
      Local<Value> argv[argc] = { Local<Value>::New(Integer::New((int) my_info->m->event)) };

      my_info->callback->Call(Context::GetCurrent()->Global(), argc, argv);
    }

  free (my_info->m);
  my_info->m = NULL;
  // and go look for the next event
  uv_queue_work(uv_default_loop(),req,do_queue_work,done_queue_work);

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
  tpl->PrototypeTemplate()->Set(String::NewSymbol("Disconnect"),
      FunctionTemplate::New(Disconnect)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("CharWriteCommand"),
      FunctionTemplate::New(CharWriteCommand)->GetFunction());

  Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Device"), constructor);

}
// Make one by calling new Device("11:22:33:44:55:66",js_callback);
Handle<Value> Device::New(const Arguments& args) {
  HandleScope scope;
  uv_work_t *req = new uv_work_t;
  struct info *my_info = new struct info;
  
  fprintf(stderr,"New device being made...\n");
  Device* obj = new Device();
  //obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->counter_ =0;
  //Local<Function> cb = Local<Function>::Cast(args[0]);

  // get the param
  v8::String::Utf8Value param1(args[0]->ToString());
  std::string text1 = std::string(*param1);
  const char * addr = text1.c_str();
  obj->m_cb = v8::Persistent<v8::Function>::New(args[1]);
  strcpy(obj->m_address,addr);


  strcpy(my_info->addr,obj->m_address);
  my_info->m = NULL;
  my_info->callback = obj->m_cb;
  req->data = my_info;
  
  uv_queue_work(uv_default_loop(),req,do_queue_work,done_queue_work);

  
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




  fprintf(stderr,"device: Coonect to %s\n",obj->m_address);

  struct messageQ *m = malloc (sizeof(struct messageQ ));
  m->event=CONNECT_OUT;
  strcpy(m->addr ,obj->m_address);
  m_glibhandler->AddEventToGLIBQ(m);

  return scope.Close(Undefined());
  
}
//disconnect method
Handle<Value> Device::Disconnect(const Arguments& args) {
  HandleScope scope;
  Device* obj = ObjectWrap::Unwrap<Device>(args.This());




  fprintf(stderr,"device: DISCoonect to %s\n",obj->m_address);

  struct messageQ *m = malloc (sizeof(struct messageQ ));
  m->event=DISCONNECT_OUT;
  strcpy(m->addr ,obj->m_address);
  m_glibhandler->AddEventToGLIBQ(m);

  return scope.Close(Undefined());
  
}


//char_write_command method
// no cb functions, so no libuv shenanigans
Handle<Value> Device::CharWriteCommand(const Arguments& args) {
  HandleScope scope;
  Device* obj = ObjectWrap::Unwrap<Device>(args.This());
  // get the param
  int handle = args[0]->NumberValue();
  uint8_t myValue = args[1]->NumberValue();
  char valueString[128];

  uint8_t *value;
  
  // work around for bluez issue. they expect a hex number in 
  // string form without any preceding 0x.  eg 20 not 0x20 
  sprintf(valueString,"%02x",myValue);
  size_t plen=gatt_attr_data_from_string(valueString, &value);
  fprintf(stderr,"valuestring = %s value = %d non gatt value = %d\n",valueString,*value,myValue); 
  
  if (plen == 0)
    {
      fprintf(stderr,"bad plen device:%s\n", obj->m_address);
      return scope.Close(Undefined());
    }

  struct messageQ *m = malloc (sizeof(struct messageQ ));
  m->event=  CHAR_WRITE_CMD_OUT;
  strcpy(m->addr ,obj->m_address);
  m->handle=handle;
  m->value=value;
  m->plen=plen;
  m_glibhandler->AddEventToGLIBQ(m);
  


  return scope.Close(Undefined());
  
}
