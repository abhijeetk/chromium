// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/android/shell_manager.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/functional/bind.h"
#include "base/lazy_instance.h"
#include "content/public/browser/web_contents.h"
#include "content/shell/android/content_shell_jni_headers/ShellManager_jni.h"
#include "content/shell/browser/shell.h"
#include "content/shell/browser/shell_browser_context.h"
#include "content/shell/browser/shell_content_browser_client.h"
#include "url/gurl.h"
#include "ui/android/window_android.h"

#if BUILDFLAG(ANATIVE_BUILD)
#include "base/android/android_app_state.h"
#include "components/embedder_support/android/view/content_view_render_view.h"
#endif

#if BUILDFLAG(ANATIVE_BUILD)
#include "components/embedder_support/android/view/content_view_render_view.h"
#include "ui/android/window_android.h"
#endif

using base::android::JavaParamRef;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;

namespace {

struct GlobalState {
  GlobalState() {}
  base::android::ScopedJavaGlobalRef<jobject> j_shell_manager;
#if BUILDFLAG(ANATIVE_BUILD)
  RAW_PTR_EXCLUSION content::ShellManager* g_shell_manager;
#endif
};

base::LazyInstance<GlobalState>::DestructorAtExit g_global_state =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace content {

ScopedJavaLocalRef<jobject> CreateShellView(Shell* shell) {
#if BUILDFLAG(ANATIVE_BUILD)
  gfx::NativeWindow window = new ui::WindowAndroid(
      /*env=*/nullptr, /*obj=*/nullptr, /*sdk_display_id=*/0,
      /*scroll_factor=*/180, /*window_is_wide_color_gamut=*/0);

  g_global_state.Get().g_shell_manager =
      new ShellManager(window, nullptr, shell);
  auto* shell_manager = g_global_state.Get().g_shell_manager;
  shell_manager->createShell(0);
#else
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_ShellManager_createShell(env,
                                       g_global_state.Get().j_shell_manager,
                                       reinterpret_cast<intptr_t>(shell));
#endif
}

void RemoveShellView(const JavaRef<jobject>& shell_view) {
#if BUILDFLAG(ANATIVE_BUILD)
// TODO(IGALIA) : Implement me.
#else
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ShellManager_removeShell(env, g_global_state.Get().j_shell_manager,
                                shell_view);
#endif
}

static void JNI_ShellManager_Init(JNIEnv* env,
                                  const JavaParamRef<jobject>& obj) {
#if BUILDFLAG(ANATIVE_BUILD)
// TODO(IGALIA) : Implement me.
#else
  g_global_state.Get().j_shell_manager.Reset(obj);
#endif
}

void JNI_ShellManager_LaunchShell(JNIEnv* env,
                                  const JavaParamRef<jstring>& jurl) {
  LOG(ERROR) << "ABHIJEET : " << __FUNCTION__;
  ShellBrowserContext* browserContext =
      ShellContentBrowserClient::Get()->browser_context();
  GURL url(base::android::ConvertJavaStringToUTF8(env, jurl));
  Shell::CreateNewWindow(browserContext, url, nullptr, gfx::Size());
}

void DestroyShellManager() {
#if BUILDFLAG(ANATIVE_BUILD)
// TODO(IGALIA) : Implement me.
#else
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ShellManager_destroy(env, g_global_state.Get().j_shell_manager);
#endif
}

#if BUILDFLAG(ANATIVE_BUILD)
const std::string DEFAULT_SHELL_URL = "http://www.google.com";

ShellManager::ShellManager(gfx::NativeWindow window,
                           embedder_support::ContentViewRenderView* view,
                           Shell* new_shell)
    : mContentViewRenderView(view),
      mWindow(window),
      mActiveShell(new_shell),
      mStartupUrl(DEFAULT_SHELL_URL) {}

ShellManager::~ShellManager() {
  destroy();
}

void ShellManager::setWindow(gfx::NativeWindow window) {
}

gfx::NativeWindow ShellManager::getWindow() const {
  return mWindow;
}

embedder_support::ContentViewRenderView*
ShellManager::getContentViewRenderView() const {
  return mContentViewRenderView;
}

void ShellManager::setStartupUrl(const std::string& url) {
  mStartupUrl = url;
}

Shell* ShellManager::getActiveShell() const {
  return mActiveShell;
}

void ShellManager::launchShell(const std::string& url) {
}

Shell* ShellManager::createShell(long nativeShellPtr) {
  if (!mContentViewRenderView) {
    mContentViewRenderView =
        new embedder_support::ContentViewRenderView(nullptr, nullptr, mWindow);
  }

  mContentViewRenderView->SurfaceCreated(nullptr, nullptr);
  mContentViewRenderView->SurfaceChanged(nullptr, nullptr, 4, 1080, 2097,
                                         nullptr);
  CHECK(mActiveShell);
  showShell(mActiveShell);

  return mActiveShell;
}

void ShellManager::showShell(Shell* shellView) {
  mActiveShell = shellView;

  // Simulating web contents behavior
  WebContents* webContents = mActiveShell->web_contents();
  CHECK(webContents);
  if (webContents) {
    mContentViewRenderView->SetCurrentWebContents(webContents);
    webContents->WasShown();
  }
}

void ShellManager::removeShell(Shell* shellView) {
}

void ShellManager::destroy() {
  if (mContentViewRenderView != nullptr) {
    mContentViewRenderView = nullptr;
  }
}

#endif

}  // namespace content
