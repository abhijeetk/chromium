/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
#include <android_native_app_glue.h>

#include "base/android/android_app_state.h"
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

// Tear down the display and application.
static void term_display() {}

// Process the next input event.
static int32_t handle_input(android_app* app, AInputEvent* event) {
  return 0;
}

// Process the next main command.
static void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      LOG(ERROR) << "IGALIA : APP_CMD_SAVE_STATE";
      break;
    case APP_CMD_INIT_WINDOW:
      LOG(ERROR) << "IGALIA : APP_CMD_INIT_WINDOW";
      // The window is being shown, get it ready.

      if (app->window != nullptr) {
        base::android::InitVM(app->activity->vm);
        if (!content::android::OnJNIOnLoadInit()) {
          return;
        }

        content::ContentMainDelegate* delegate =
            new content::ShellMainDelegate();
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
      }
      break;
    case APP_CMD_TERM_WINDOW:
      LOG(ERROR) << "IGALIA : APP_CMD_TERM_WINDOW";
      break;
    case APP_CMD_GAINED_FOCUS:
      LOG(ERROR) << "IGALIA : APP_CMD_GAINED_FOCUS";
      break;
    case APP_CMD_LOST_FOCUS:
      LOG(ERROR) << "IGALIA : APP_CMD_LOST_FOCUS";
      break;
    case APP_CMD_WINDOW_RESIZED:
      LOG(ERROR) << "IGALIA : APP_CMD_WINDOW_RESIZED";
      break;
    default:
      break;
  }
}

void android_main(android_app* state) {
  // Update the app state in base/android/android_app_state.h
  g_native_app_state = state;

  state->onAppCmd = handle_cmd;
  state->onInputEvent = handle_input;
  
  while (!state->destroyRequested) {
    // Our input, sensor, and update/render logic is all driven by callbacks, so
    // we don't need to use the non-blocking poll.
    android_poll_source* source = nullptr;
    auto result = ALooper_pollOnce(-1, nullptr, nullptr,
                                   reinterpret_cast<void**>(&source));
    if (result == ALOOPER_POLL_ERROR) {
      LOG(ERROR) << "ALooper_pollOnce returned an error";
    }

    if (source != nullptr) {
      source->process(state, source);
    }
  }

  term_display();
}
