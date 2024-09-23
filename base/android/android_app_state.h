#ifndef BASE_ANDROID_APP_STATE_H_
#define BASE_ANDROID_APP_STATE_H_

#include <android_native_app_glue.h>
#include "base/base_export.h"

// TODO(abhijeet) : Replace the global variable with a Singleton implementation
// to ensure controlled access to the instance and to enforce a single instance
// across the application

BASE_EXPORT extern struct android_app* g_native_app_state;

#endif  // BASE_ANDROID_APP_STATE_H_
