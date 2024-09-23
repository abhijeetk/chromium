// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <jni.h>

#include "base/android/apk_assets.h"

#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/base_jni/ApkAssets_jni.h"
#include "base/debug/crash_logging.h"
#include "base/debug/dump_without_crashing.h"
#include "base/file_descriptor_store.h"

#if BUILDFLAG(ANATIVE_BUILD)
#include <android/asset_manager.h>
#include "base/android/android_app_state.h"
#include "base/logging.h"
#endif

namespace base {
namespace android {

int OpenApkAsset(const std::string& file_path_input,
                 const std::string& split_name,
                 base::MemoryMappedFile::Region* region) {
  std::string file_path = file_path_input;
#if BUILDFLAG(ANATIVE_BUILD)
  if (!g_native_app_state) {
    return -1;
  }
  // Find the position of the substring and remove it
  std::string to_remove = "assets/";
  size_t pos = file_path.find(to_remove);
  if (pos != std::string::npos) {
    file_path.erase(pos, to_remove.length());
  }

  AAssetManager* asset_manager = g_native_app_state->activity->assetManager;
  // Open the asset
  AAsset* asset =
      AAssetManager_open(asset_manager, file_path.c_str(), AASSET_MODE_UNKNOWN);
  if (!asset) {
    LOG(ERROR) << "AssetManager : Failed to open asset: " << file_path.c_str();
    return -1;  // Failed to open the asset
  }

  // Get the asset size
  long asset_size = AAsset_getLength(asset);
  if (asset_size <= 0) {
    AAsset_close(asset);
    LOG(ERROR) << "AssetManager : Asset size is invalid: " << file_path.c_str();
    return -1;
  }

  // Get the file descriptor and its associated offset/size
  long asset_offset = 0;

  // Map the asset into memory (we don't have direct file descriptors, so just
  // use the asset)
  int fd = AAsset_openFileDescriptor(asset, &asset_offset, &asset_size);
  if (fd < 0) {
    LOG(ERROR) << "AssetManager : Failed to get file descriptor for asset: "
               << file_path.c_str();
    AAsset_close(asset);
    return -1;
  }

  // Assign values to region
  region->offset = static_cast<off_t>(asset_offset);
  region->size = static_cast<size_t>(asset_size);

  AAsset_close(asset);
  return fd;  // Return the file descriptor of the asset

#else
  // The AssetManager API of the NDK does not expose a method for accessing
  // raw resources :(
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jlongArray> jarr =
      Java_ApkAssets_open(env, ConvertUTF8ToJavaString(env, file_path),
                          ConvertUTF8ToJavaString(env, split_name));
  std::vector<jlong> results;
  base::android::JavaLongArrayToLongVector(env, jarr, &results);
  CHECK_EQ(3U, results.size());
  int fd = static_cast<int>(results[0]);
  region->offset = results[1];
  // Not a checked_cast because open() may return -1.
  region->size = static_cast<size_t>(results[2]);
  return fd;
#endif
}

int OpenApkAsset(const std::string& file_path,
                 base::MemoryMappedFile::Region* region) {
  return OpenApkAsset(file_path, std::string(), region);
}

bool RegisterApkAssetWithFileDescriptorStore(const std::string& key,
                                             const base::FilePath& file_path) {
  base::MemoryMappedFile::Region region =
      base::MemoryMappedFile::Region::kWholeFile;
  int asset_fd = OpenApkAsset(file_path.value(), &region);
  if (asset_fd == -1)
    return false;
  base::FileDescriptorStore::GetInstance().Set(key, base::ScopedFD(asset_fd),
                                               region);
  return true;
}

void DumpLastOpenApkAssetFailure() {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> error =
      Java_ApkAssets_takeLastErrorString(env);
  if (!error) {
    return;
  }
  SCOPED_CRASH_KEY_STRING256("base", "OpenApkAssetError",
                             ConvertJavaStringToUTF8(env, error));
  base::debug::DumpWithoutCrashing();
}

}  // namespace android
}  // namespace base
