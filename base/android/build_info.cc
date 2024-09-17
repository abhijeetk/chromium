// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/build_info.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/scoped_java_ref.h"
#include "base/base_jni/BuildInfo_jni.h"
#include "base/check_op.h"
#include "base/memory/singleton.h"
#include "base/notreached.h"
#include "base/strings/string_number_conversions.h"
#include "build/blink_buildflags.h"

namespace base {
namespace android {

namespace {

// We are leaking these strings.
const char* StrDupParam(const std::vector<std::string>& params, size_t index) {
  return strdup(params[index].c_str());
}

int GetIntParam(const std::vector<std::string>& params, size_t index) {
  int ret = 0;
  bool success = StringToInt(params[index], &ret);
  DCHECK(success);
  return ret;
}

}  // namespace

struct BuildInfoSingletonTraits {
  static BuildInfo* New() {
    std::vector<std::string> params;
#if !BUILDFLAG(SNAP_BUILD)
    JNIEnv* env = AttachCurrentThread();
    ScopedJavaLocalRef<jobjectArray> params_objs = Java_BuildInfo_getAll(env);
    AppendJavaStringArrayToStringVector(env, params_objs, &params);
#else
    params.push_back("samsung");
    params.push_back("m14x");
    params.push_back("UP1A.231005.007");
    params.push_back("samsung");
    params.push_back("SM-M146B");
    params.push_back("34");
    params.push_back("user");
    params.push_back("s5e8535");
    params.push_back("org.chromium.content_shell_apk");
    params.push_back("1");
    params.push_back("Content Shell");
    params.push_back("org.chromium.content_shell_apk");
    params.push_back("1");
    params.push_back("Developer Build");
    params.push_back(
        "samsung/m14xnnxx/m14x:14/UP1A.231005.007/M146BXXS6CXH1:user/"
        "release-keys");
    params.push_back("243433038");
    params.push_back("");
    params.push_back("arm64-v8a, armeabi-v7a, armeabi");
    params.push_back("false");
    params.push_back("Not Enabled");
    params.push_back("34");
    params.push_back("0");
    params.push_back("0");
    params.push_back("M146BXXS6CXH1");
    params.push_back("s5e8535");
    params.push_back("1");
    params.push_back("0");
    params.push_back("1");
    params.push_back("1");
    params.push_back("REL");
    params.push_back("132514561");
    params.push_back("0");
#endif

    return new BuildInfo(params);
  }

  static void Delete(BuildInfo* x) {
    // We're leaking this type, see kRegisterAtExit.
    NOTREACHED();
  }

  static const bool kRegisterAtExit = false;
#if DCHECK_IS_ON()
  static const bool kAllowedToAccessOnNonjoinableThread = true;
#endif
};

BuildInfo::BuildInfo(const std::vector<std::string>& params)
    : brand_(StrDupParam(params, 0)),
      device_(StrDupParam(params, 1)),
      android_build_id_(StrDupParam(params, 2)),
      manufacturer_(StrDupParam(params, 3)),
      model_(StrDupParam(params, 4)),
      sdk_int_(GetIntParam(params, 5)),
      build_type_(StrDupParam(params, 6)),
      board_(StrDupParam(params, 7)),
      host_package_name_(StrDupParam(params, 8)),
      host_version_code_(StrDupParam(params, 9)),
      host_package_label_(StrDupParam(params, 10)),
      package_name_(StrDupParam(params, 11)),
      package_version_code_(StrDupParam(params, 12)),
      package_version_name_(StrDupParam(params, 13)),
      android_build_fp_(StrDupParam(params, 14)),
      gms_version_code_(StrDupParam(params, 15)),
      installer_package_name_(StrDupParam(params, 16)),
      abi_name_(StrDupParam(params, 17)),
      custom_themes_(StrDupParam(params, 18)),
      resources_version_(StrDupParam(params, 19)),
      target_sdk_version_(GetIntParam(params, 20)),
      is_debug_android_(GetIntParam(params, 21)),
      is_tv_(GetIntParam(params, 22)),
      version_incremental_(StrDupParam(params, 23)),
      hardware_(StrDupParam(params, 24)),
      is_at_least_t_(GetIntParam(params, 25)),
      is_automotive_(GetIntParam(params, 26)),
      is_at_least_u_(GetIntParam(params, 27)),
      targets_at_least_u_(GetIntParam(params, 28)),
      codename_(StrDupParam(params, 29)),
      vulkan_deqp_level_(GetIntParam(params, 30)),
      is_foldable_(GetIntParam(params, 31)) {}

// static
BuildInfo* BuildInfo::GetInstance() {
  return Singleton<BuildInfo, BuildInfoSingletonTraits >::get();
}

}  // namespace android
}  // namespace base
