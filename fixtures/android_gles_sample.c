#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// Android NDK Bionic / Log declarations
extern int __android_log_print(int prio, const char *tag, const char *fmt, ...);

// EGL & OpenGL ES 2.0 declarations
typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLSurface;
typedef void* ANativeWindow;

#define EGL_NONE 0x3038
#define EGL_RED_SIZE 0x3024
#define EGL_GREEN_SIZE 0x3023
#define EGL_BLUE_SIZE 0x3022
#define EGL_ALPHA_SIZE 0x3021
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_OPENGL_ES2_BIT 4
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define GL_COLOR_BUFFER_BIT 0x00004000

extern EGLDisplay eglGetDisplay(void* native_display);
extern int eglInitialize(EGLDisplay dpy, int *major, int *minor);
extern int eglChooseConfig(EGLDisplay dpy, const int *attrib_list, EGLConfig *configs, int config_size, int *num_config);
extern EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, ANativeWindow win, const int *attrib_list);
extern EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const int *attrib_list);
extern int eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
extern int eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
extern void glClearColor(float r, float g, float b, float a);
extern void glClear(uint32_t mask);

typedef struct ANativeActivityCallbacks {
    void (*onStart)(void* activity);
    void (*onResume)(void* activity);
    void* (*onSaveInstanceState)(void* activity, size_t* outSize);
    void (*onPause)(void* activity);
    void (*onStop)(void* activity);
    void (*onDestroy)(void* activity);
    void (*onWindowFocusChanged)(void* activity, int hasFocus);
    void (*onNativeWindowCreated)(void* activity, ANativeWindow window);
    void (*onNativeWindowResized)(void* activity, ANativeWindow window);
    void (*onNativeWindowRedrawNeeded)(void* activity, ANativeWindow window);
    void (*onNativeWindowDestroyed)(void* activity, ANativeWindow window);
    void (*onInputQueueCreated)(void* activity, void* queue);
    void (*onInputQueueDestroyed)(void* activity, void* queue);
    void (*onContentRectChanged)(void* activity, const void* rect);
    void (*onConfigurationChanged)(void* activity);
    void (*onLowMemory)(void* activity);
} ANativeActivityCallbacks;

typedef struct ANativeActivity {
    ANativeActivityCallbacks* callbacks;
    void* vm;
    void* env;
    void* clazz;
    void* internalDataPath;
    void* externalDataPath;
    int32_t sdkVersion;
    void* instance;
    void* assetManager;
    void* obbPath;
} ANativeActivity;

static void on_native_window_created(void* activity, ANativeWindow window) {
    __android_log_print(4, "Maarch64GLES", "[ANativeActivity] onNativeWindowCreated called with handle %p", window);

    EGLDisplay dpy = eglGetDisplay(NULL);
    int maj = 0, min = 0;
    eglInitialize(dpy, &maj, &min);

    int attribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig config = NULL;
    int num_config = 0;
    eglChooseConfig(dpy, attribs, &config, 1, &num_config);

    EGLSurface surface = eglCreateWindowSurface(dpy, config, window, NULL);
    int ctx_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLContext ctx = eglCreateContext(dpy, config, NULL, ctx_attribs);

    eglMakeCurrent(dpy, surface, surface, ctx);

    // Clear background to vibrant modern blue (R: 0.1, G: 0.4, B: 0.9)
    glClearColor(0.1f, 0.4f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(dpy, surface);
    __android_log_print(4, "Maarch64GLES", "[ANativeActivity] SUCCESS: OpenGL ES 2.0 Frame rendered to Host Desktop Window!");
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    __android_log_print(4, "Maarch64GLES", "[ANativeActivity_onCreate] Initializing Android Native Activity...");
    if (activity && activity->callbacks) {
        activity->callbacks->onNativeWindowCreated = on_native_window_created;
    }
}
