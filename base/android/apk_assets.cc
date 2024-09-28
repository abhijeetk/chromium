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
#include "base/android/android_app_state.h"
#include <android/asset_manager.h>
#include "base/logging.h"
#endif

namespace base {
namespace android {

#if BUILDFLAG(ANATIVE_BUILD)
static int android_read(void* asset, char* buf, int size) {
    return AAsset_read((AAsset*)asset, buf, size);
}

static int android_write(void* asset, const char* buf, int size) {
    return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek(void* asset, fpos_t offset, int whence) {
    return AAsset_seek((AAsset*)asset, offset, whence);
}

static int android_close(void* asset) {
    AAsset_close((AAsset*)asset);
    return 0;
}


FILE* android_fopen(const char* fname, const char* mode, AAssetManager *assetManager) {
    if(mode[0] == 'w') return nullptr;

    AAsset* asset = AAssetManager_open( assetManager, fname, 0);
    if(!asset) return nullptr;

    return funopen(asset, android_read, android_write, android_seek, android_close);
}

//We will eventually improve this, but let's settle with a familiar structure, for now.
std::string readShaderToString( FILE* fileDescriptor ) {
    const unsigned N=1024;
    std::string total;
    while (true) {
        char buffer[ N ];
        size_t read = fread((void *)&buffer[0], 1, N, fileDescriptor);
        if (read) {
            for ( int c = 0; c <  read; ++c ) {
                total.push_back( buffer[ c ] );
            }
        }
        if (read < N) {
            break;
        }
    }
    LOG(ERROR) << "Size : " << total.length();
    return total;
}
#endif

int OpenApkAsset(const std::string& file_path_input,
                 const std::string& split_name,
                 base::MemoryMappedFile::Region* region) {
  std::string file_path = file_path_input;
  // LOG(ERROR) <<"<------------------------|||--------------------------->" << g_native_app_state;
#if BUILDFLAG(ANATIVE_BUILD)
  if (!g_native_app_state) {
    return -1;
  }

  // LOG(ERROR) <<"<------------------------|||--------------------------->";
  // Find the position of the substring and remove it
  std::string to_remove = "assets/";
  size_t pos = file_path.find(to_remove);
  if (pos != std::string::npos) {
    file_path.erase(pos, to_remove.length());
  }

  AAssetManager* asset_manager = g_native_app_state->activity->assetManager;
  // Open the asset
  AAsset* asset =
      AAssetManager_open(asset_manager, file_path.c_str(), AASSET_MODE_STREAMING);
  if (!asset) {
    // LOG(ERROR) <<"AssetManager : Failed to open asset: " << file_path.c_str();
    return -1;  // Failed to open the asset
  }

  // Get the asset size
  long asset_size = AAsset_getLength(asset);
  if (asset_size <= 0) {
    AAsset_close(asset);
    // LOG(ERROR) <<"AssetManager : Asset size is invalid: " << file_path.c_str();
    return -1;
  }

  // Get the file descriptor and its associated offset/size
  long asset_offset = 0;

  // Map the asset into memory (we don't have direct file descriptors, so just
  // use the asset)
  int fd = AAsset_openFileDescriptor(asset, &asset_offset, &asset_size);
  if (fd < 0) {
    // LOG(ERROR) <<"AssetManager : Failed to get file descriptor for asset: "
    //           << file_path.c_str();
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
