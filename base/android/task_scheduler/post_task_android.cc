// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/task_scheduler/post_task_android.h"

#include "base/base_jni/PostTask_jni.h"
#include "base/logging.h"
#include "build/blink_buildflags.h"

namespace base {

// static
void PostTaskAndroid::SignalNativeSchedulerReady() {
#if BUILDFLAG(ANATIVE_BUILD)
  LOG(ERROR) << "TODO(IGALIA)) : Implement " << __FUNCTION__
             << " for android native build";
#else
  Java_PostTask_onNativeSchedulerReady(jni_zero::AttachCurrentThread());
#endif
}

}  // namespace base
