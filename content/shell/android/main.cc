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

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>

namespace content {
namespace {

ContentMainRunner* GetContentMainRunner() {
  static base::NoDestructor<std::unique_ptr<ContentMainRunner>> runner{
      ContentMainRunner::Create()};
  return runner->get();
}
}  // namespace
}  // namespace content

EGLDisplay display;
EGLSurface surface;
EGLContext context;

int32_t handle_input(android_app* app, AInputEvent* event) {
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    // Handle touch events
    float x = AMotionEvent_getX(event, 0);
    float y = AMotionEvent_getY(event, 0);
    // Process the touch event (e.g., update object positions)
    return 1;  // Event was handled
  }
  return 0;  // Event was not handled
}

void init_graphics(ANativeWindow* window) {
  const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                            EGL_BLUE_SIZE,    8,
                            EGL_GREEN_SIZE,   8,
                            EGL_RED_SIZE,     8,
                            EGL_NONE};
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config = nullptr;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, nullptr, nullptr);

  /* Here, the application chooses the configuration it desires.
   * find the best match if possible, otherwise use the very first one
   */
  eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);
  std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
  assert(supportedConfigs);
  eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs,
                  &numConfigs);
  assert(numConfigs);
  auto i = 0;
  for (; i < numConfigs; i++) {
    auto& cfg = supportedConfigs[i];
    EGLint r, g, b, d;
    if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r) &&
        eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
        eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b) &&
        eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) && r == 8 &&
        g == 8 && b == 8 && d == 0) {
      config = supportedConfigs[i];
      break;
    }
  }
  if (i == numConfigs) {
    config = supportedConfigs[0];
  }

  if (config == nullptr) {
    LOG(ERROR) << "ABHIJEET : Unable to initialize EGLConfig";
    return;
  }

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
  surface = eglCreateWindowSurface(display, config, window, nullptr);

  /* A version of OpenGL has not been specified here.  This will default to
   * OpenGL 1.0.  You will need to change this if you want to use the newer
   * features of OpenGL like shaders. */
  context = eglCreateContext(display, config, nullptr, nullptr);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    LOG(ERROR) << "ABHIJEET : Unable to eglMakeCurrent";
    return;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  LOG(ERROR) << "ABHIJEET : display : " << display << "\n"
             << "context : " << context << "\n"
             << "surface : " << surface << "\n"
             << "w : " << w << "\n"
             << "h : " << h << "\n";

  // Check openGL on the system
  auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
  for (auto name : opengl_info) {
    auto info = glGetString(name);
    LOG(ERROR) << "ABHIJEET : OpenGL Info : " << info;
  }
  // Initialize GL state.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_DEPTH_TEST);

  return;
#if 0
  // Initialize EGL (OpenGL ES) context and surface
  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display == EGL_NO_DISPLAY) {
    // Handle the error: Display is invalid
    LOG(ERROR) << "ABHIJEET : Failed to get EGL display";
    CHECK(false);
  }

  if (!eglInitialize(display, nullptr, nullptr)) {
    // Handle the error: Failed to initialize EGL display
    LOG(ERROR) << "ABHIJEET : Failed to initialize EGL display";
    CHECK(false);
  }

  EGLConfig config;
  EGLint numConfigs;
  const EGLint configAttributes[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                     EGL_BLUE_SIZE,    8,
                                     EGL_GREEN_SIZE,   8,
                                     EGL_RED_SIZE,     8,
                                     EGL_NONE};

  eglChooseConfig(display, configAttributes, &config, 1, &numConfigs);

  if (numConfigs == 0) {
    LOG(ERROR) << "ABHIJEET : Failed to choose EGL config";
    CHECK(false);
    return;
  }

  LOG(ERROR) << "ABHIJEET : <------- numConfigs ----------> : " << numConfigs;

  surface = eglCreateWindowSurface(display, config, window, nullptr);
  if (surface == EGL_NO_SURFACE) {
    LOG(ERROR) << "ABHIJEET : Failed to create EGL window surface";
    CHECK(false);
    return;
  }

  context = eglCreateContext(display, config, EGL_NO_CONTEXT, nullptr);
  if (context == EGL_NO_CONTEXT) {
    LOG(ERROR) << "ABHIJEET : Failed to create EGL context";
    CHECK(false);
    return;
  }

  if (!eglMakeCurrent(display, surface, surface, context)) {
    LOG(ERROR) << "ABHIJEET : Failed to make EGL context current";
    CHECK(false);
    return;
  }
#endif
}

void render() {
  // LOG(ERROR) << "ABHIJEET : ABHIJEET :  : ";
  // Clear the screen and set a background color
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw your objects using OpenGL ES commands here

  // Swap buffers to display the current frame
  eglSwapBuffers(display, surface);
}

void cleanup_graphics() {
  eglDestroySurface(display, surface);
  eglDestroyContext(display, context);
  eglTerminate(display);
}

void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (app->window != nullptr) {
        LOG(ERROR) << "ABHIJEET : ABHIJEET : window : " << app->window
                   << "\tg_native_app_state : " << g_native_app_state->window;
        // Initialize graphics (e.g., OpenGL ES) and prepare to draw
        init_graphics(app->window);

        LOG(ERROR) << "ABHIJEET : Abhijeet : externalDataPath : "
                   << app->activity->externalDataPath;
        LOG(ERROR) << "ABHIJEET : Abhijeet : internalDataPath : "
                   << app->activity->internalDataPath;
        LOG(ERROR) << "ABHIJEET : Abhijeet : assetManager : "
                   << app->activity->assetManager;
        LOG(ERROR) << "ABHIJEET : Abhijeet : env : " << app->activity->env;
        LOG(ERROR) << "ABHIJEET : Abhijeet : vm : " << app->activity->vm;
        LOG(ERROR) << "ABHIJEET : Abhijeet : sdkVersion : " << app->activity->sdkVersion;

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
      LOG(ERROR) << "ABHIJEET : ABHIJEET : window : " << app->window;
      // Clean up resources when the window is closed
      cleanup_graphics();
      break;
      // Handle other cases such as window resizing or pausing
  }
}

void android_main(android_app* app) {
  g_native_app_state = app;

#if 0

  }
#endif

  base::android::InitVM(app->activity->vm);
  if (!content::android::OnJNIOnLoadInit()) {
    return;
  }

  // Set the callback to handle commands like APP_CMD_INIT_WINDOW
  app->onAppCmd = handle_cmd;
  app->onInputEvent = handle_input;

  // Main loop
  int events;
  android_poll_source* source;

  while (true) {
    while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
      if (source) {
        source->process(app, source);
      }

      if (app->destroyRequested != 0) {
        cleanup_graphics();
        return;
      }
    }

    // Application-specific rendering and update logic here
    render();
  }
}
// END_INCLUDE(all)
