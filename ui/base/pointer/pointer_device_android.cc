// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/pointer/pointer_device.h"

#include "base/android/jni_array.h"
#include "base/check_op.h"
#include "ui/base/ui_base_jni_headers/TouchDevice_jni.h"
#include "base/logging.h"

using jni_zero::AttachCurrentThread;

namespace ui {

TouchScreensAvailability GetTouchScreensAvailability() {
  return TouchScreensAvailability::ENABLED;
}

int MaxTouchPoints() {
#if 0
  int touch_points = Java_TouchDevice_maxTouchPoints(AttachCurrentThread());
  LOG(ERROR) << "IGALIA : touch_points " << touch_points;
#endif
  return 5; //Java_TouchDevice_maxTouchPoints(AttachCurrentThread());
}

std::pair<int, int> AvailablePointerAndHoverTypes() {
#if 0 //BUILDFLAG(ANATIVE_BUILD)
  JNIEnv* env = AttachCurrentThread();
  std::vector<int> pointer_and_hover_types;
  base::android::JavaIntArrayToIntVector(
      env, Java_TouchDevice_availablePointerAndHoverTypes(env),
      &pointer_and_hover_types);
  DCHECK_EQ(pointer_and_hover_types.size(), 2u);
  LOG(ERROR) << "pointer_and_hover_types[0] : " << pointer_and_hover_types[0] << "\tpointer_and_hover_types[1] : " << pointer_and_hover_types[1];
  return std::make_pair(pointer_and_hover_types[0], pointer_and_hover_types[1]);
#else
  return std::make_pair(6, 2);
#endif
}

int GetAvailableHoverTypes() {
  return AvailablePointerAndHoverTypes().second;
}

int GetAvailablePointerTypes() {
  return AvailablePointerAndHoverTypes().first;
}

PointerType GetPrimaryPointerType(int available_pointer_types) {
  if (available_pointer_types & POINTER_TYPE_COARSE)
    return POINTER_TYPE_COARSE;
  if (available_pointer_types & POINTER_TYPE_FINE)
    return POINTER_TYPE_FINE;
  DCHECK_EQ(available_pointer_types, POINTER_TYPE_NONE);
  return POINTER_TYPE_NONE;
}

HoverType GetPrimaryHoverType(int available_hover_types) {
  if (available_hover_types & HOVER_TYPE_NONE)
    return HOVER_TYPE_NONE;
  DCHECK_EQ(available_hover_types, HOVER_TYPE_HOVER);
  return HOVER_TYPE_HOVER;
}

}  // namespace ui
