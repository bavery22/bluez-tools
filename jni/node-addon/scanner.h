#ifndef SCANNER_H
#define SCANNER_H

#include <node.h>

class Scanner : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);

 private:
  Scanner();
  ~Scanner();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
  static v8::Handle<v8::Value> LEScan (const v8::Arguments& args);
  static v8::Handle<v8::Value> CWR (const v8::Arguments& args);
  double counter_;
  v8::Persistent<v8::Function> m_cb;
};

#endif
