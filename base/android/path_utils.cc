// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/path_utils.h"

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/files/file_path.h"

#include "base/base_jni/PathUtils_jni.h"

#if BUILDFLAG(ANATIVE_BUILD)
#include "base/android/android_app_state.h"
#endif

namespace base {
namespace android {

bool GetDataDirectory(FilePath* result) {
#if BUILDFLAG(ANATIVE_BUILD)
  if (g_native_app_state) {
    FilePath data_path("/data/user/0/com.example.myapp/app_content_shell");
    *result = data_path;
    return true;
  }
  return false;
#else
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> path = Java_PathUtils_getDataDirectory(env);
  FilePath data_path(ConvertJavaStringToUTF8(path));
  *result = data_path;
  return true;
#endif
}

bool GetCacheDirectory(FilePath* result) {
#if BUILDFLAG(ANATIVE_BUILD)
  if (g_native_app_state) {
    FilePath cache_path("/data/user/0/com.example.myapp/cache");
    *result = cache_path;
    return true;
  }
  return false;
#else
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> path = Java_PathUtils_getCacheDirectory(env);
  FilePath cache_path(ConvertJavaStringToUTF8(path));
  *result = cache_path;
  return true;
#endif
}

bool GetThumbnailCacheDirectory(FilePath* result) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> path =
      Java_PathUtils_getThumbnailCacheDirectory(env);
  FilePath thumbnail_cache_path(ConvertJavaStringToUTF8(path));
  *result = thumbnail_cache_path;
  return true;
}

bool GetDownloadsDirectory(FilePath* result) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> path = Java_PathUtils_getDownloadsDirectory(env);
  FilePath downloads_path(ConvertJavaStringToUTF8(path));
  *result = downloads_path;
  return true;
}

std::vector<FilePath> GetAllPrivateDownloadsDirectories() {
  std::vector<std::string> dirs;
  JNIEnv* env = AttachCurrentThread();
  auto jarray = Java_PathUtils_getAllPrivateDownloadsDirectories(env);
  base::android::AppendJavaStringArrayToStringVector(env, jarray, &dirs);

  std::vector<base::FilePath> file_paths;
  for (const auto& dir : dirs)
    file_paths.emplace_back(dir);

  return file_paths;
}

std::vector<FilePath> GetSecondaryStorageDownloadDirectories() {
  std::vector<std::string> dirs;
  JNIEnv* env = AttachCurrentThread();
  auto jarray = Java_PathUtils_getExternalDownloadVolumesNames(env);
  base::android::AppendJavaStringArrayToStringVector(env, jarray, &dirs);

  std::vector<base::FilePath> file_paths;
  for (const auto& dir : dirs)
    file_paths.emplace_back(dir);
  return file_paths;
}

bool GetNativeLibraryDirectory(FilePath* result) {
#if BUILDFLAG(ANATIVE_BUILD)
  if (g_native_app_state) {
    FilePath library_path(
        "/data/app/~~mCREp3eQwaHPrC8YoUD0sQ==/"
        "com.example.myapp-TVqioX1NN-6qFmT-8IJSOw==/lib/arm64");
    *result = library_path;
    return true;
  }
  return false;
#else
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> path =
      Java_PathUtils_getNativeLibraryDirectory(env);
  FilePath library_path(ConvertJavaStringToUTF8(path));
  *result = library_path;
  return true;
#endif
}

bool GetExternalStorageDirectory(FilePath* result) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> path =
      Java_PathUtils_getExternalStorageDirectory(env);
  FilePath storage_path(ConvertJavaStringToUTF8(path));
  *result = storage_path;
  return true;
}

}  // namespace android
}  // namespace base
