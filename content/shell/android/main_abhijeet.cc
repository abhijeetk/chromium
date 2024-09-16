/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
#include <android_native_app_glue.h>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "content/app/android/content_main_android.h"
#include "content/browser/android/content_startup_flags.h"
#include "content/public/app/content_jni_onload.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_delegate.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/common/content_client.h"
#include "content/shell/app/shell_main_delegate.h"

extern struct android_app* g_app_state;

// START : TEST
#include "base/files/memory_mapped_file.h"
// This function opens an asset and maps it into memory.
int OpenApkAsset(AAssetManager* asset_manager, const std::string& file_path, 
                 const std::string& split_name, base::MemoryMappedFile::Region* region) {
    // Open the asset
    AAsset* asset = AAssetManager_open(asset_manager, file_path.c_str(), AASSET_MODE_UNKNOWN);
    if (!asset) {
        LOG(ERROR) << "AssetManager : Failed to open asset: " << file_path.c_str();
        return -1; // Failed to open the asset
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

    // Map the asset into memory (we don't have direct file descriptors, so just use the asset)
    int fd = AAsset_openFileDescriptor(asset, &asset_offset, &asset_size);
    if (fd < 0) {
        LOG(ERROR) << "AssetManager : Failed to get file descriptor for asset: " << file_path.c_str();
        AAsset_close(asset);
        return -1;
    }

    // Assign values to region
    region->offset = static_cast<off_t>(asset_offset);
    region->size = static_cast<size_t>(asset_size);

    AAsset_close(asset);
    return fd;  // Return the file descriptor of the asset
}
// END : TEST

namespace content {
namespace {

ContentMainRunner* GetContentMainRunner() {
  static base::NoDestructor<std::unique_ptr<ContentMainRunner>> runner{
      ContentMainRunner::Create()};
  return runner->get();
}
}  // namespace
}  // namespace content

void android_main(android_app* state) {
  g_app_state = state;
  LOG(ERROR) << "----- START : ABHIJEET ----- ";
  LOG(ERROR) << "externalDataPath : " << state->activity->externalDataPath;
  LOG(ERROR) << "internalDataPath : " << state->activity->internalDataPath;
  LOG(ERROR) << "assetManager : " << state->activity->assetManager;
  LOG(ERROR) << "env : " << state->activity->env;
  LOG(ERROR) << "vm : " << state->activity->vm;
  LOG(ERROR) << "sdkVersion : " << state->activity->sdkVersion;

#if 0
  AAssetManager* asset_manager = state->activity->assetManager;
  base::MemoryMappedFile::Region region;
  int fd = OpenApkAsset(asset_manager, "content_shell.pak", "", &region);

  if (fd >= 0) {
      // You can now memory-map the asset or perform operations on it using the file descriptor.
      LOG(ERROR) << "File opened : ";
      close(fd);
  }

  return;
#endif

  base::android::InitVM(state->activity->vm);

  if (!content::android::OnJNIOnLoadInit()) {
    return;
  }

  content::ContentMainDelegate* delegate = new content::ShellMainDelegate();
  content::SetContentMainDelegate(delegate);


  static const char* const kInitialArgv[] = {"MativeActivity"};
  base::CommandLine::Init(std::size(kInitialArgv), kInitialArgv);

  // Java: BrowserStartupControllerJavaImpl::prepareToStartBrowserProcess
  //       - BrowserStartupControllerImplJni.get().setCommandLineFlags
  content::SetContentCommandLineFlags(/*singleProcess=*/false);

  content::ContentMainParams params(delegate);
  params.minimal_browser_mode = false;

  content::RunContentProcess(std::move(params),
                             content::GetContentMainRunner());

  [[maybe_unused]] base::CommandLine* command_line(
      base::CommandLine::ForCurrentProcess());
  LOG(ERROR) << "commandline : " << __FUNCTION__ << "\t" << command_line->GetArgumentsString();

  while (!state->destroyRequested) {
    // Our input, sensor, and update/render logic is all driven by callbacks, so
    // we don't need to use the non-blocking poll.
    android_poll_source* source = nullptr;
    auto result = ALooper_pollOnce(-1, nullptr, nullptr,
                                   reinterpret_cast<void**>(&source));
    if (result == ALOOPER_POLL_ERROR) {
      LOG(ERROR) << "ALooper_pollOnce returned an error";
    }
  }
  LOG(ERROR) << "----- END : ABHIJEET ----- ";
}
// END_INCLUDE(all)
