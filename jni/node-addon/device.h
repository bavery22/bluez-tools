#ifndef DEVICE_H
#define DEVICE_H

#include <node.h>
#include <string>
#include "glibHandler.h"



class Device : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);

 private:
  Device();
  ~Device();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
  static v8::Handle<v8::Value> CWR (const v8::Arguments& args);
  static v8::Handle<v8::Value> Connect (const v8::Arguments& args);
  static v8::Handle<v8::Value> CharWriteCommand (const v8::Arguments& args);
  double counter_;
  char m_address[20];
  v8::Persistent<v8::Function> m_cb;



  // singleton glibhandler
  static GlibHandler* m_glibhandler;

};



#endif
