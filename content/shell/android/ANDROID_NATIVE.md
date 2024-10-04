### args.gn
Belows args are used to build android_webview, content_shell and android native.
```
target_os = "android"
target_cpu = "arm64"
#target_cpu = "x64"
use_remoteexec = true
is_debug = true
# reclient_cfg_dir = "../../buildtools/reclient_cfgs/linux" # for latest code
rbe_cfg_dir = "../../buildtools/reclient_cfgs/linux" 
#is_official_build = true
ffmpeg_branding="Chrome"
proprietary_codecs=true
use_siso = true
is_java_debug = true
#system_webview_package_name = "com.google.android.webview.dev"
system_webview_shell_package_name = "org.chromium.my_webview_shell"
use_signing_keys = true
#target_cpu="x64"
#root_extra_deps = ["//tools/gn/tutorial:hello_world", "//tools/gn/tutorial:hello", "//tools/gn/tutorial:say_hello_linux"]
# exclude_unwind_tables = false # Optional, allows to get stack traces on release builds.
is_robolectric = false
treat_warnings_as_errors = false
#enable_vulkan = falsei
enable_gpu_client_logging=true
enable_gpu_service_logging=true
anative_build=true
```

### Build and Run command 
- Build and run android native application
```
anative.sh && ./out/android-arm64-rel/bin/content_shell_apk_anative run 
```

- Build and run content_shell application
```
content_shell.sh && ./out/android-arm64-rel/bin/content_shell_apk run
```


Download [anative.sh](https://github.com/abhijeetk/chromium/blob/native-activity-using-gn-launch-shell_02/android-native/anative.sh) and [content_shell.sh](https://github.com/abhijeetk/chromium/blob/native-activity-using-gn-launch-shell_02/android-native/content_shell.sh) 