/**
 * Copyright (c) Jonathan Cardoso Machado. All Rights Reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <napi.h>
#include <uv.h>

namespace ReproduceIssue {

class ReproduceIssue : public Napi::ObjectWrap<ReproduceIssue> {
public:
  ReproduceIssue(const Napi::CallbackInfo &info);
  ~ReproduceIssue();
  static Napi::Function Init(Napi::Env env, Napi::Object exports);
  Napi::Value OnMessage(const Napi::CallbackInfo &info);
  Napi::Value Close(const Napi::CallbackInfo &info);

  bool isOpen = true;

private:
  void CloseTimerAsync();
  void Dispose();
  void CallOnMessageCallback();

  Napi::FunctionReference cbOnMessage;
  std::unique_ptr<Napi::AsyncContext> asyncContextOnMessage;

  uv_timer_t timeout;
  bool timerClosed = false;
  napi_async_cleanup_hook_handle removeHandle;

  static void OnTimeout(uv_timer_t *timer);
  static void CleanupHookAsync(napi_async_cleanup_hook_handle handle,
                               void *data);

  ReproduceIssue(const ReproduceIssue &that) = delete;
  ReproduceIssue &operator=(const ReproduceIssue &that) = delete;
};

} // namespace ReproduceIssue
