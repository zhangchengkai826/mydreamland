//
// Created by andys on 5/10/2019.
//

#include <engine.h>

static Engine engine;

static pthread_t renderLoopTID, physicsLoopTID;
static pthread_mutex_t renderLoopShouldExit, physicsLoopShouldExit;
static pthread_mutex_t engineMutex;
static bool bRenderLoopThreadRunning = false;

static void *physicsLoop(void *arg) {
    while(true) {
        if(pthread_mutex_trylock(&physicsLoopShouldExit) == 0) {
            // lock succeed
            pthread_mutex_unlock(&physicsLoopShouldExit);
            break;
        };

        // do main tasks here
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "### physicsLoop ###");
        timespec tm{
                .tv_sec = 1,
                .tv_nsec = 0,
        };
        nanosleep(&tm, nullptr);

        int fpsFrameCounter;
        pthread_mutex_lock(&engineMutex);
        fpsFrameCounter = engine.fpsFrameCounter;
        engine.fpsFrameCounter = 0;
        pthread_mutex_unlock(&engineMutex);
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "fps: %d", fpsFrameCounter);
    }
    return nullptr;
}

static void *renderLoop(void *arg) {
    while(true) {
        if(pthread_mutex_trylock(&renderLoopShouldExit) == 0) {
            // lock succeed
            pthread_mutex_unlock(&renderLoopShouldExit);
            if(engine.bDisplayInited) {
                engine.destroyDisplay();
                engine.bDisplayInited = false;
            }
            break;
        };

        ANativeWindow *window;
        pthread_mutex_lock(&engineMutex);
        window = engine.window;
        pthread_mutex_unlock(&engineMutex);

        if(!engine.bDisplayInited && window) {
            engine.initDisplay();
            engine.bDisplayInited = true;
        } else if(engine.bDisplayInited && !window) {
            engine.destroyDisplay();
            engine.bDisplayInited = false;
        }

        // do main tasks here
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "### renderLoop ###");

        if(engine.bDisplayInited && engine.bAnimating) {
            engine.drawFrame();

            pthread_mutex_lock(&engineMutex);
            engine.fpsFrameCounter++;
            pthread_mutex_unlock(&engineMutex);
        } else {
            timespec tm{
                .tv_sec = 0,
                .tv_nsec = 100000000,
            };
            nanosleep(&tm, nullptr);
        }
    }
    return nullptr;
}

static void ANativeActivity_onStart(ANativeActivity *activity) {
    __android_log_print(ANDROID_LOG_INFO, "main", "### ANativeActivity_onStart ###");

    /* global initialize, only main threads exists */

    engine.activity = activity;
    engine.init();
    engine.bDisplayInited = false;

    pthread_mutex_init(&engineMutex, nullptr);
    pthread_mutex_init(&renderLoopShouldExit, nullptr);
    pthread_mutex_init(&physicsLoopShouldExit, nullptr);

    pthread_mutex_lock(&renderLoopShouldExit);
    pthread_mutex_lock(&physicsLoopShouldExit);
    bRenderLoopThreadRunning = true;

    /* global initialize end */

    pthread_create(&renderLoopTID, nullptr, renderLoop, nullptr);
    pthread_create(&physicsLoopTID, nullptr, physicsLoop, nullptr);
}

// make sure all resources are freed, all threads are joined
static void ANativeActivity_onStop(ANativeActivity *activity) {
    __android_log_print(ANDROID_LOG_INFO, "main", "### ANativeActivity_onStop ###");

    pthread_mutex_unlock(&renderLoopShouldExit);
    pthread_mutex_unlock(&physicsLoopShouldExit);
    pthread_join(renderLoopTID, nullptr);
    pthread_join(physicsLoopTID, nullptr);

    /* global destroy, only main thread exists */

    bRenderLoopThreadRunning = false;

    pthread_mutex_destroy(&renderLoopShouldExit);
    pthread_mutex_destroy(&physicsLoopShouldExit);
    pthread_mutex_destroy(&engineMutex);

    engine.destroy();
    engine.activity = nullptr;

    /* global destroy end */
}

static void ANativeActivity_onNativeWindowCreated(ANativeActivity *activity,
        ANativeWindow *window) {
    __android_log_print(ANDROID_LOG_INFO, "main",
            "### ANativeActivity_onNativeWindowCreated ###");

    if(bRenderLoopThreadRunning) {
        /* engineMutex is initialized before bRenderLoopThreadRunning is set to true */
        /* bRenderLoopThreadRunning is set to true before renderLoop starts */
        pthread_mutex_lock(&engineMutex);
        engine.window = window;
        pthread_mutex_unlock(&engineMutex);
    } else {
        engine.window = window;
    }
}

static void ANativeActivity_onNativeWindowDestroyed(ANativeActivity *activity,
        ANativeWindow *window) {
    __android_log_print(ANDROID_LOG_INFO, "main",
            "### ANativeActivity_onNativeWindowDestroyed ###");

    if(bRenderLoopThreadRunning) {
        /* bRenderLoopThreadRunning is set to false before engineMutex is destroyed */
        /* renderLoop stops before bRenderLoopThreadRunning is set to false */
        pthread_mutex_lock(&engineMutex);
        engine.window = nullptr;
        pthread_mutex_unlock(&engineMutex);
    } else {
        engine.window = nullptr;
    }
}

static void ANativeActivity_onResume(ANativeActivity* activity) {
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "### ANativeActivity_onResume ###");

    pthread_mutex_lock(&engineMutex);
    engine.bAnimating = true;
    pthread_mutex_unlock(&engineMutex);
}

static void ANativeActivity_onPause(ANativeActivity* activity) {
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "### ANativeActivity_onPause ###");

    pthread_mutex_lock(&engineMutex);
    engine.bAnimating = false;
    pthread_mutex_unlock(&engineMutex);
}

JNIEXPORT
void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState,
                              size_t savedStateSize) {
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "### ANativeActivity_onCreate ###");

    jclass jclass_NativeActivity = activity->env->GetObjectClass(activity->clazz);
    jmethodID jmethod_NativeActivity_getClassLoader = activity->env->GetMethodID(
            jclass_NativeActivity, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject jclass_ClassLoader_obj = activity->env->CallObjectMethod(activity->clazz,
            jmethod_NativeActivity_getClassLoader);
    jclass jclass_ClassLoader = activity->env->FindClass("java/lang/ClassLoader");
    jmethodID jmethod_ClassLoader_loadClass = activity->env->GetMethodID(jclass_ClassLoader,
            "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring strClassName = activity->env->NewStringUTF("com.mydreamland.www.Util");
    auto jclass_Util = static_cast<jclass>(activity->env->CallObjectMethod(jclass_ClassLoader_obj,
            jmethod_ClassLoader_loadClass, strClassName));
    activity->env->DeleteLocalRef(strClassName);
    jmethodID jmethod_Util_static_checkPermission = activity->env->GetStaticMethodID(jclass_Util,
            "checkPermission", "(Landroid/app/NativeActivity;)V");
    activity->env->CallStaticVoidMethod(jclass_Util, jmethod_Util_static_checkPermission,
            activity->clazz);

    activity->callbacks->onStart = ANativeActivity_onStart;
    activity->callbacks->onStop = ANativeActivity_onStop;
    activity->callbacks->onNativeWindowCreated = ANativeActivity_onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = ANativeActivity_onNativeWindowDestroyed;
    activity->callbacks->onResume = ANativeActivity_onResume;
    activity->callbacks->onPause = ANativeActivity_onPause;
}
