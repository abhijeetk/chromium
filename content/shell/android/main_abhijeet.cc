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
  LOG(ERROR) << "----- START : ABHIJEET ----- ";
  // LOG(ERROR) << state->activity->externalDataPath;
  // LOG(ERROR) << state->activity->internalDataPath;

  base::android::InitVM(state->activity->vm);

  if (!content::android::OnJNIOnLoadInit()) {
    return;
  }

  content::ContentMainDelegate* delegate = new content::ShellMainDelegate();
  content::SetContentMainDelegate(delegate);

  static const char* const kInitialArgv[] = {"ChromeTestActivity"};
  base::CommandLine::Init(std::size(kInitialArgv), kInitialArgv);

  content::ContentMainParams params(delegate);
  params.minimal_browser_mode = false;

  content::RunContentProcess(std::move(params),
                             content::GetContentMainRunner());

  [[maybe_unused]] base::CommandLine* command_line(
      base::CommandLine::ForCurrentProcess());
  LOG(ERROR) << "commandline : " << command_line->GetArgumentsString();

  while (!state->destroyRequested) {
    // Our input, sensor, and update/render logic is all driven by callbacks, so
    // we don't need to use the non-blocking poll.
    android_poll_source* source = nullptr;
    auto result = ALooper_pollOnce(-1, nullptr, nullptr,
                                   reinterpret_cast<void**>(&source));
    if (result == ALOOPER_POLL_ERROR) {
      LOG(ERROR) << "ALooper_pollOnce returned an error";
    }

    LOG(ERROR) << "----- END : ABHIJEET ----- ";
  }
}
// END_INCLUDE(all)
