#include <node.h>
#include "scanner.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
using namespace v8;

// a list of devices that we will add to as we find them
struct devices{
  char *gEntries[20];
  int numEntries;
  int timeToScan;
  v8::Persistent<v8::Function> callback;

};

static void do_work(uv_work_t *req)
{
  fprintf(stderr,"do_work\n");
  struct devices *d = (struct devices *) req->data;

  FILE *fp;
  int status;
  char path[1035];

  //fp = popen("/system/xbin/hcitool-btle lescan","r");
  //fp = popen("ls  /system/etc/dhcpcd","r");
  //fp = popen("/system/xbin/hcitool-btle frog 2>&1 ","r");
  char buf[128];
  snprintf(buf,128,"/system/bin/hcitool-btle lescan --time=%d 2>&1 ",d->timeToScan);
  //fp = popen("/system/xbin/hcitool-btle lescan --time=5 2>&1 ","r");
  fp = popen(buf,"r");
  fprintf(stderr,"do_work buf= = %s\n",buf);
  while ((d->numEntries < 20) && (fgets(path, sizeof(path)-1, fp) != NULL)) {
  //while (fgets(path, sizeof(path)-1, fp) != NULL) {
    printf("do_work> str=%s of size %d\n", path,strlen(path));
    
    d->gEntries[d->numEntries]=malloc(strlen(path)+1);
    strcpy(d->gEntries[d->numEntries++],path);
    printf("copy is %s\n",d->gEntries[d->numEntries-1]);
  }
  fprintf(stderr,"do_work closing the fp. sad.\n");
  pclose(fp);

}
static void done_work(uv_work_t *req, int status)
{
  fprintf(stderr,"done_work   status = %d\n");
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
}


Scanner::Scanner() {};
Scanner::~Scanner() {};

void Scanner::Init(Handle<Object> target) {

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Scanner"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
      FunctionTemplate::New(PlusOne)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("LEScan"),
      FunctionTemplate::New(LEScan)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("CWR"),
      FunctionTemplate::New(CWR)->GetFunction());

  Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Scanner"), constructor);

}

Handle<Value> Scanner::New(const Arguments& args) {
  HandleScope scope;

  Scanner* obj = new Scanner();
  //obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->counter_ =0;
  //Local<Function> cb = Local<Function>::Cast(args[0]);
  obj->m_cb = v8::Persistent<v8::Function>::New(args[0]);
  /*
const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("hello world")) };
  obj->CB->Call(Context::GetCurrent()->Global(), argc, argv);
  */
  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> Scanner::PlusOne(const Arguments& args) {
  HandleScope scope;

  Scanner* obj = ObjectWrap::Unwrap<Scanner>(args.This());
  obj->counter_ += 1;
  // call the callback
  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("cows")) };
  obj->m_cb->Call(Context::GetCurrent()->Global(), argc, argv);
  
  return scope.Close(Number::New(obj->counter_));
}

Handle<Value> Scanner::LEScan(const Arguments& args) {
  HandleScope scope;
  Scanner* obj = ObjectWrap::Unwrap<Scanner>(args.This());
  uv_work_t *req = new uv_work_t;
  struct devices *my_devices = new struct devices;
  
  fprintf(stderr,"LEScan c++\n");
  my_devices->callback = obj->m_cb;
  my_devices->numEntries=0;
  my_devices->timeToScan  = args[0]->IsUndefined() ? 5 : args[0]->NumberValue();
  req->data = my_devices;
  fprintf(stderr,"LEScan c++ for %d seconds\n",my_devices->timeToScan);
  uv_queue_work(uv_default_loop(),req,do_work,done_work);
  
  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("QUEUEED")) };
  obj->m_cb->Call(Context::GetCurrent()->Global(), argc, argv);
  


  return scope.Close(Undefined());

}

//char-write-req
Handle<Value> Scanner::CWR(const Arguments& args) {
  HandleScope scope;

  Scanner* obj = ObjectWrap::Unwrap<Scanner>(args.This());
  //  my_devices->timeToScan  = args[0]->IsUndefined() ? 5 : args[0]->NumberValue();

  return scope.Close(Undefined());
  
}
