// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/android/browser_startup_controller.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "build/blink_buildflags.h"
#include "content/browser/android/content_startup_flags.h"
#include "content/browser/browser_main_loop.h"
#include "content/public/android/content_main_dex_jni/BrowserStartupControllerImpl_jni.h"
#include "content/shell/android/shell_manager.h"

using base::android::JavaParamRef;

namespace content {

void BrowserStartupComplete(int result) {
  LOG(ERROR) << "TODO(abhijeet) : Implement " << __FUNCTION__
             << " for Android Native build : result : " << result;
  //base::debug::StackTrace().Print();
#if !BUILDFLAG(ANATIVE_BUILD)
  //ANativeGlobal::g_global_state.Get().j_shell_manager.Reset(nullptr);
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_BrowserStartupControllerImpl_browserStartupComplete(env, result);
#else
  //launchNativeShell("https://www.igalia.com");
  //LOG(ERROR) << "TODO(abhijeet) : Implement " << __FUNCTION__
  //           << " for Android Native build : g_global_state : " << ANativeGlobal::g_global_state.Get().g_shell_manager;
#endif
}

void MinimalBrowserStartupComplete() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_BrowserStartupControllerImpl_minimalBrowserStartupComplete(env);
}

bool ShouldStartGpuProcessOnBrowserStartup() {
#if !BUILDFLAG(ANATIVE_BUILD)
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_BrowserStartupControllerImpl_shouldStartGpuProcessOnBrowserStartup(
      env);
#else
  return true;
#endif
}

static void JNI_BrowserStartupControllerImpl_SetCommandLineFlags(
    JNIEnv* env,
    jboolean single_process) {
  SetContentCommandLineFlags(static_cast<bool>(single_process));
}

static void JNI_BrowserStartupControllerImpl_FlushStartupTasks(JNIEnv* env) {
  BrowserMainLoop::GetInstance()->SynchronouslyFlushStartupTasks();
}

}  // namespace content
