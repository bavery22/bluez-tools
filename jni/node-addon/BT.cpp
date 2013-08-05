#include <node.h>
#include "scanner.h"
#include "device.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Scanner::Init(exports);
  Device::Init(exports);
}

NODE_MODULE(BT, InitAll)
