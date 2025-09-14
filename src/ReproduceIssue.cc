#include "ReproduceIssue.h"
#include "uv.h"

namespace ReproduceIssue {

// Constructor
ReproduceIssue::ReproduceIssue(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<ReproduceIssue>(info) {
  Napi::Env env = info.Env();

  uv_loop_t *loop = nullptr;
  auto napi_result = napi_get_uv_event_loop(env, &loop);
  if (napi_result != napi_ok) {
    throw Napi::Error::New(env, "Failed to get UV event loop.");
  }

  uv_timer_init(loop, &this->timeout);
  this->timeout.data = this;
  this->Ref();

  napi_add_async_cleanup_hook(env, ReproduceIssue::CleanupHookAsync, this,
                              &removeHandle);

  uv_timer_start(&this->timeout, ReproduceIssue::OnTimeout, 1000, 0);
}

void ReproduceIssue::CleanupHookAsync(napi_async_cleanup_hook_handle handle,
                                      void *data) {
  ReproduceIssue *reproduceIssue = static_cast<ReproduceIssue *>(data);
  reproduceIssue->CloseTimerAsync();
}

void ReproduceIssue::CloseTimerAsync() {
  if (this->timerClosed) {
    return;
  }

  uv_handle_t *timeoutHandle = reinterpret_cast<uv_handle_t *>(&this->timeout);
  if (!uv_is_closing(timeoutHandle)) {
    uv_timer_stop(&this->timeout);

    uv_close(timeoutHandle, [](uv_handle_t *handle) {
      uv_timer_t *timer = reinterpret_cast<uv_timer_t *>(handle);
      ReproduceIssue *reproduceIssue =
          static_cast<ReproduceIssue *>(timer->data);
      napi_remove_async_cleanup_hook(reproduceIssue->removeHandle);
      reproduceIssue->Unref();
    });
    this->timerClosed = true;
  }
}

ReproduceIssue::~ReproduceIssue() {
  if (this->isOpen) {
    this->Dispose();
  }
}

void ReproduceIssue::Dispose() {
  if (!this->isOpen)
    return;

  this->isOpen = false;

  uv_timer_stop(&this->timeout);

  this->cbOnMessage.Reset();
}

// Initialize the class for export
Napi::Function ReproduceIssue::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func =
      DefineClass(env, "ReproduceIssue",
                  {
                      // Instance methods
                      InstanceMethod("onMessage", &ReproduceIssue::OnMessage),
                      InstanceMethod("close", &ReproduceIssue::Close),
                  });

  exports.Set("ReproduceIssue", func);

  return func;
}

Napi::Value ReproduceIssue::OnMessage(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!info.Length()) {
    throw Napi::TypeError::New(
        env,
        "You must specify the callback function. If you want to remove the "
        "current one you can pass null.");
  }

  Napi::Value arg = info[0];
  bool isNull = arg.IsNull();

  if (!arg.IsFunction() && !isNull) {
    throw Napi::TypeError::New(
        env,
        "Argument must be a Function. If you want to remove the current one "
        "you can pass null.");
  }

  if (isNull) {
    this->cbOnMessage.Reset();
    this->asyncContextOnMessage.reset();
  } else {
    this->cbOnMessage = Napi::Persistent(arg.As<Napi::Function>());

    this->asyncContextOnMessage =
        std::make_unique<Napi::AsyncContext>(env, "ReproduceIssue::OnMessage");
  }

  return info.This();
}

Napi::Value ReproduceIssue::Close(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!this->isOpen) {
    throw Napi::TypeError::New(env, "ReproduceIssue handle already closed.");
  }

  this->Dispose();

  return env.Undefined();
}

void ReproduceIssue::CallOnMessageCallback() {
  if (this->cbOnMessage.IsEmpty())
    return;
  if (!this->isOpen)
    return;

  Napi::Env env = Env();
  Napi::HandleScope scope(env);

  const char *useMakeCallback = std::getenv("USE_MAKE_CALLBACK");
  bool shouldUseMakeCallback =
      useMakeCallback && std::string(useMakeCallback) == "true";

  Napi::Function callback = this->cbOnMessage.Value();

  Napi::Value data = env.Null();

  try {
    if (shouldUseMakeCallback) {
      callback.MakeCallback(this->Value(), {data},
                            *this->asyncContextOnMessage);
    } else {
      callback.Call(this->Value(), {data});
    }
  } catch (const Napi::Error &e) {
    // ignore any js error
  }

  if (!this->isOpen)
    return;
}

void ReproduceIssue::OnTimeout(uv_timer_t *timer) {
  ReproduceIssue *obj = static_cast<ReproduceIssue *>(timer->data);

  obj->CallOnMessageCallback();
}

} // namespace ReproduceIssue
