#include <android/log.h>
#include <android_native_app_glue.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))

static char g_app_dir[PATH_MAX];

// Structure to hold process configuration
struct ProcessConfig {
    const char* process_type;
    const char* process_dir;
    const char* custom_android_data;  // Allow custom ANDROID_DATA path if needed
};

bool create_process_directory(const char* dir_path) {
    if (mkdir(dir_path, 0711) == -1 && errno != EEXIST) {
        LOGE("Failed to create directory %s: %s", dir_path, strerror(errno));
        return false;
    }
    return true;
}

bool setup_process_environment(const ProcessConfig* config) {
    // Create process-specific directory
    char process_dir[PATH_MAX];
    snprintf(process_dir, sizeof(process_dir), "%s/%s", g_app_dir, config->process_dir);
    
    if (!create_process_directory(process_dir)) {
        return false;
    }

    // Create tmp directory
    char tmp_dir[PATH_MAX];
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", process_dir);
    if (!create_process_directory(tmp_dir)) {
        return false;
    }

    // Set TMPDIR environment variable
    if (setenv("TMPDIR", tmp_dir, 1) == -1) {
        LOGE("Failed to set TMPDIR: %s", strerror(errno));
        return false;
    }

    // Set ANDROID_DATA
    const char* android_data = config->custom_android_data ? config->custom_android_data : "/data";
    if (setenv("ANDROID_DATA", android_data, 1) == -1) {
        LOGE("Failed to set ANDROID_DATA: %s", strerror(errno));
        return false;
    }

    return true;
}

bool set_process_name(const char* process_type) {
    if (prctl(PR_SET_NAME, process_type, 0, 0, 0) == -1) {
        LOGE("Failed to set process name: %s", strerror(errno));
        return false;
    }
    return true;
}

bool get_app_directory(struct android_app* app) {
    if (!app || !app->activity || !app->activity->internalDataPath) {
        LOGE("Invalid app state");
        return false;
    }
    
    snprintf(g_app_dir, sizeof(g_app_dir), "%s", app->activity->internalDataPath);
    LOGI("App internal path: %s", g_app_dir);
    return true;
}

bool launch_child_process(const ProcessConfig* config) {
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        LOGE("Failed to read /proc/self/exe: %s", strerror(errno));
        return false;
    }
    exe_path[len] = '\0';
    
    pid_t pid = fork();
    if (pid == -1) {
        LOGE("Fork failed: %s", strerror(errno));
        return false;
    }

    if (pid == 0) {
        // Child process
        LOGI("Launching %s process", config->process_type);

        // Set up process environment before anything else
        if (!setup_process_environment(config)) {
            _exit(1);
        }

        // Set the process name
        if (!set_process_name(config->process_type)) {
            _exit(1);
        }

        // Set up environment variables
        char ld_library_path[PATH_MAX * 2];
        snprintf(ld_library_path, sizeof(ld_library_path), "LD_LIBRARY_PATH=%s/lib", g_app_dir);

        // Get current ANDROID_DATA value
        const char* android_data = getenv("ANDROID_DATA");
        if (!android_data) {
            android_data = "/data";  // Fallback to default if not set
        }
        char android_data_env[PATH_MAX];
        snprintf(android_data_env, sizeof(android_data_env), "ANDROID_DATA=%s", android_data);

        char android_root[PATH_MAX];
        snprintf(android_root, sizeof(android_root), "ANDROID_ROOT=/system");

        char* envp[] = {
            ld_library_path,
            android_data_env,
            android_root,
            NULL
        };

        // Arguments
        char* args[] = {
            exe_path,
            (char*)config->process_type,
            NULL
        };

        LOGI("Executing %s process with %s", config->process_type, exe_path);
        LOGI("ANDROID_DATA=%s", android_data);
        
        execve(exe_path, args, envp);
        LOGE("execve failed: %s", strerror(errno));
        _exit(1);
    }
    else {
        // Parent process
        LOGI("Parent waiting for child %s process (PID: %d)", config->process_type, pid);
        
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            LOGI("Child %s process exited with status %d", config->process_type, WEXITSTATUS(status));
            return WEXITSTATUS(status) == 0;
        } else if (WIFSIGNALED(status)) {
            LOGE("Child %s process terminated by signal %d", config->process_type, WTERMSIG(status));
        }
    }
    
    return false;
}

void android_main(struct android_app* app) {
    if (!get_app_directory(app)) {
        LOGE("Failed to get app directory");
        return;
    }

    LOGI("Launching child processes");

    // Define process configurations
    const ProcessConfig renderer_config = {
        .process_type = "--process=renderer",
        .process_dir = "renderer",
        .custom_android_data = "/data"  // Use system Android data directory
    };

    const ProcessConfig graphics_config = {
        .process_type = "--process=graphics",
        .process_dir = "graphics",
        .custom_android_data = "/data"  // Use system Android data directory
    };

    // Launch processes
    if (!launch_child_process(&renderer_config)) {
        LOGE("Failed to launch renderer process");
        return;
    }

    if (!launch_child_process(&graphics_config)) {
        LOGE("Failed to launch graphics process");
        return;
    }

    LOGI("Main process running");
    
    while (1) {
        int events;
        struct android_poll_source* source;
        
        while (ALooper_pollAll(0, NULL, &events, (void**)&source) >= 0) {
            if (source != NULL) {
                source->process(app, source);
            }
            
            if (app->destroyRequested != 0) {
                LOGI("Destroy requested, cleaning up");
                return;
            }
        }
    }
}