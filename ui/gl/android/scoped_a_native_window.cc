// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gl/android/scoped_a_native_window.h"

#include <android/native_window_jni.h>

#include "base/android/jni_android.h"
#include "build/blink_buildflags.h"
#include "ui/gl/android/scoped_java_surface.h"

#if BUILDFLAG(ANATIVE_BUILD)
#include "base/android/android_app_state.h"
#endif

namespace gl {

// static
ScopedANativeWindow ScopedANativeWindow::Wrap(ANativeWindow* a_native_window) {
  return ScopedANativeWindow(a_native_window);
}

ScopedANativeWindow::ScopedANativeWindow(const ScopedJavaSurface& surface) {
#if BUILDFLAG(ANATIVE_BUILD)
  a_native_window_ = g_native_app_state->window;
#else
  if (!surface.j_surface()) {
    return;
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  // Note: This ensures that any local references used by
  // ANativeWindow_fromSurface are released immediately. This is needed as a
  // workaround for https://code.google.com/p/android/issues/detail?id=68174
  base::android::ScopedJavaLocalFrame scoped_local_reference_frame(env);
  a_native_window_ = ANativeWindow_fromSurface(env, surface.j_surface().obj());
#endif
}

ScopedANativeWindow::ScopedANativeWindow(ANativeWindow* a_native_window)
    : a_native_window_(a_native_window) {
  if (a_native_window_) {
    ANativeWindow_acquire(a_native_window_);
  }
}

ScopedANativeWindow::~ScopedANativeWindow() {
  DestroyIfNeeded();
}

ScopedANativeWindow::ScopedANativeWindow(ScopedANativeWindow&& other)
    : a_native_window_(other.a_native_window_) {
  other.a_native_window_ = nullptr;
}

ScopedANativeWindow& ScopedANativeWindow::operator=(
    ScopedANativeWindow&& other) {
  if (this != &other) {
    DestroyIfNeeded();
    a_native_window_ = other.a_native_window_;
    other.a_native_window_ = nullptr;
  }
  return *this;
}

void ScopedANativeWindow::DestroyIfNeeded() {
  if (a_native_window_) {
    ANativeWindow_release(a_native_window_);
  }
  a_native_window_ = nullptr;
}

}  // namespace gl
