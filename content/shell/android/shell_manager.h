// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_ANDROID_SHELL_MANAGER_H_
#define CONTENT_SHELL_ANDROID_SHELL_MANAGER_H_

#include <jni.h>

#include "build/blink_buildflags.h"
#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

class Shell;

#if BUILDFLAG(ANATIVE_BUILD)
#include <string>
#include "ui/gfx/native_widget_types.h"

namespace embedder_support {
class ContentViewRenderView;
}
#endif

namespace cc {
class Layer;
}

namespace content {

class Shell;

// Creates an Android specific shell view, which is our version of a shell
// window.  This view holds the controls and content views necessary to
// render a shell window.  Returns the java object representing the shell view.
// object.
base::android::ScopedJavaLocalRef<jobject> CreateShellView(Shell* shell);

// Removes a previously created shell view.
void RemoveShellView(const base::android::JavaRef<jobject>& shell_view);

void ShellAttachLayer(cc::Layer* layer);
void ShellRemoveLayer(cc::Layer* layer);

// Destroys the ShellManager on app exit. Must not use the above functions
// after this is called.
void DestroyShellManager();

#if BUILDFLAG(ANATIVE_BUILD)
class ShellManager {
 public:
  ShellManager(gfx::NativeWindow window,
               embedder_support::ContentViewRenderView* view,
               Shell* shell);
  ~ShellManager();

  ShellManager(const ShellManager&) = delete;
  ShellManager& operator=(const ShellManager&) = delete;

  void setWindow(gfx::NativeWindow window);
  void setStartupUrl(const std::string& url);

  gfx::NativeWindow getWindow() const;
  embedder_support::ContentViewRenderView* getContentViewRenderView() const;
  Shell* getActiveShell() const;

  void launchShell(const std::string& url);
  Shell* createShell(long nativeShellPtr);
  void showShell(Shell* shellView);
  void removeShell(Shell* shellView);
  void destroy();

 private:
  gfx::NativeWindow mWindow = nullptr;  // Internally points to WindowAndroid*
  RAW_PTR_EXCLUSION Shell* mActiveShell = nullptr;
  RAW_PTR_EXCLUSION embedder_support::ContentViewRenderView*
      mContentViewRenderView = nullptr;
  std::string mStartupUrl;
};
#endif

}  // namespace content

#endif  // CONTENT_SHELL_ANDROID_SHELL_MANAGER_H_
