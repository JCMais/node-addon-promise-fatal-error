#include "ReproduceIssue.h"
#include <napi.h>

namespace ReproduceIssue {

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  ReproduceIssue::Init(env, exports);
  return exports;
}

NODE_API_MODULE(addon, InitAll)

} // namespace ReproduceIssue
