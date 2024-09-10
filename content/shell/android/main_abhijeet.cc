#include <android/log.h>
#include <android_native_app_glue.h>
#include "content/public/app/content_main.h"
#include "content/shell/app/shell_main_delegate.h"

#include "base/android/library_loader/library_loader_hooks.h"
#include "base/functional/bind.h"
#include "content/public/app/content_jni_onload.h"

#if 1
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/choreographer.h>
#include <android/log.h>
#include <android/sensor.h>
#include <android/set_abort_message.h>
#include <jni.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>
#endif

#define LOG_TAG "native-activity"

/**
#endif

 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
// #include "base/android/binder.h"
// #include "base/android/binder_box.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_android.h"
#include "base/no_destructor.h"
#include "base/trace_event/trace_event.h"
#include "content/app/android/content_main_android.h"
#include "content/browser/android/content_startup_flags.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_delegate.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/common/content_client.h"

#if 1
namespace content {
namespace {

ContentMainRunner* GetContentMainRunner() {
  static base::NoDestructor<std::unique_ptr<ContentMainRunner>> runner{
      ContentMainRunner::Create()};
  return runner->get();
}
}  // namespace
}  // namespace content
#endif

extern struct android_app* g_app_state;

void android_main(android_app* state) {
  LOG(ERROR) << "----- START : ABHIJEET ----- ";

  // Set the global app state
  // g_app_state = state;
  base::android::InitVM(state->activity->vm);
#if 1
  if (!content::android::OnJNIOnLoadInit()) {
    return;
  }

  LOG(ERROR) << "----- ABHIJEET : AFTER loading ----- ";
  content::ContentMainDelegate* delegate = new content::ShellMainDelegate();
  content::SetContentMainDelegate(delegate);

  static const char* const kInitialArgv[] = {"ChromeTestActivity"};
  base::CommandLine::Init(std::size(kInitialArgv), kInitialArgv);

  [[maybe_unused]] base::CommandLine* command_line(
      base::CommandLine::ForCurrentProcess());
  LOG(ERROR) << "commandline : " << command_line->GetArgumentsString();

  content::SetContentCommandLineFlags(false);

  content::ContentMainParams params(delegate);
  params.minimal_browser_mode = false;
  content::RunContentProcess(std::move(params),
                             content::GetContentMainRunner());
  return;
#endif
  LOG(ERROR) << "----- END : ABHIJEET ----- ";
}
// END_INCLUDE(all)
