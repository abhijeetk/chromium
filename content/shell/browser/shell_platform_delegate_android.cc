// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/browser/shell_platform_delegate.h"

#include <jni.h>

#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/command_line.h"
#include "base/containers/contains.h"
#include "base/notreached.h"
#include "base/strings/string_piece.h"
#include "build/blink_buildflags.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/shell/android/content_shell_jni_headers/Shell_jni.h"
#include "content/shell/android/shell_manager.h"
#include "content/shell/browser/shell.h"

#if BUILDFLAG(ANATIVE_BUILD)
#include "base/android/android_app_state.h"
#endif

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaParamRef;
using base::android::ScopedJavaLocalRef;

namespace content {

struct ShellPlatformDelegate::ShellData {
  base::android::ScopedJavaGlobalRef<jobject> java_object;
  RAW_PTR_EXCLUSION ANativeWindow* window;
};

struct ShellPlatformDelegate::PlatformData {};

ShellPlatformDelegate::ShellPlatformDelegate() = default;

void ShellPlatformDelegate::Initialize(const gfx::Size& default_window_size) {
}

#if !BUILDFLAG(IS_ANDROID)
gfx::NativeWindow ShellPlatformDelegate::GetNativeWindow(Shell* shell) {
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  return gfx::NativeWindow(shell_data.window);
}
#endif

ShellPlatformDelegate::~ShellPlatformDelegate() {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  DestroyShellManager();
}

void ShellPlatformDelegate::CreatePlatformWindow(
    Shell* shell,
    const gfx::Size& initial_size) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__
             << "\tinitial_size : " << initial_size.ToString();
  // base::debug::StackTrace().Print();
  DCHECK(!base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];
#if BUILDFLAG(ANATIVE_BUILD)
  shell_data.window = g_native_app_state->window;
  shell_data.java_object.Reset(CreateShellView(shell));
#else
  shell_data.java_object.Reset(CreateShellView(shell));
#endif
}

void ShellPlatformDelegate::CleanUp(Shell* shell) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  RemoveShellView(shell_data.java_object);

  if (!shell_data.java_object.is_null())
    Java_Shell_onNativeDestroyed(env, shell_data.java_object);

  shell_data_map_.erase(shell);
}

void ShellPlatformDelegate::SetContents(Shell* shell) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  //base::debug::StackTrace().Print();

#if !BUILDFLAG(ANATIVE_BUILD)
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];
  Java_Shell_initFromNativeTabContents(
      env, shell_data.java_object, shell->web_contents()->GetJavaWebContents());
#else
  // TODO(IGALIA): Implement me.
  auto* webcontent = shell->web_contents();
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__ << webcontent;
#endif
}

void ShellPlatformDelegate::ResizeWebContent(Shell* shell,
                                             const gfx::Size& content_size) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  shell->web_contents()->GetRenderWidgetHostView()->SetSize(content_size);
}

void ShellPlatformDelegate::EnableUIControl(Shell* shell,
                                            UIControl control,
                                            bool is_enabled) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  if (shell_data.java_object.is_null())
    return;
  Java_Shell_enableUiControl(env, shell_data.java_object, control, is_enabled);
}

void ShellPlatformDelegate::SetAddressBarURL(Shell* shell, const GURL& url) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  ScopedJavaLocalRef<jstring> j_url = ConvertUTF8ToJavaString(env, url.spec());
  Java_Shell_onUpdateUrl(env, shell_data.java_object, j_url);
}

void ShellPlatformDelegate::SetIsLoading(Shell* shell, bool loading) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__ << "\tloading : " << loading;
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  //Java_Shell_setIsLoading(env, shell_data.java_object, loading);
}

void ShellPlatformDelegate::SetTitle(Shell* shell,
                                     const std::u16string& title) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
}

void ShellPlatformDelegate::MainFrameCreated(Shell* shell) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  //base::debug::StackTrace().Print();
}

bool ShellPlatformDelegate::DestroyShell(Shell* shell) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  return false;  // Shell destroys itself.
}

void ShellPlatformDelegate::ToggleFullscreenModeForTab(
    Shell* shell,
    WebContents* web_contents,
    bool enter_fullscreen) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  Java_Shell_toggleFullscreenModeForTab(env, shell_data.java_object,
                                        enter_fullscreen);
}

bool ShellPlatformDelegate::IsFullscreenForTabOrPending(
    Shell* shell,
    const WebContents* web_contents) const {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  const ShellData& shell_data = shell_data_map_.find(shell)->second;

  return false; //Java_Shell_isFullscreenForTabOrPending(env, shell_data.java_object);
}

void ShellPlatformDelegate::SetOverlayMode(Shell* shell,
                                           bool use_overlay_mode) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  JNIEnv* env = base::android::AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  return Java_Shell_setOverlayMode(env, shell_data.java_object,
                                   use_overlay_mode);
}

void ShellPlatformDelegate::LoadProgressChanged(Shell* shell, double progress) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  JNIEnv* env = AttachCurrentThread();
  DCHECK(base::Contains(shell_data_map_, shell));
  ShellData& shell_data = shell_data_map_[shell];

  //Java_Shell_onLoadProgressChanged(env, shell_data.java_object, progress);
}

// static
void JNI_Shell_CloseShell(JNIEnv* env, jlong shellPtr) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  Shell* shell = reinterpret_cast<Shell*>(shellPtr);
  shell->Close();
}

}  // namespace content
