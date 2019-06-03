//
// Created by andys on 5/10/2019.
//

#include <engine.h>

static Engine engine;

static pthread_t mainLoopTID;
static pthread_mutex_t mainLoopShouldExit;
static pthread_mutex_t engineMutex;
static bool bMainLoopThreadRunning = false;

static void *mainLoop(void *arg) {
    while(true) {
        pthread_mutex_lock(&engineMutex);

        if(pthread_mutex_trylock(&mainLoopShouldExit) == 0) {
            // lock succeed
            pthread_mutex_unlock(&mainLoopShouldExit);
            if(engine.bDisplayInited) {
                engine.destroyDisplay();
                engine.bDisplayInited = false;
            }
            break;
        };

        if(!engine.bDisplayInited && engine.window) {
            engine.initDisplay();
            engine.bDisplayInited = true;
        }
        if(engine.bDisplayInited && !engine.window) {
            engine.destroyDisplay();
            engine.bDisplayInited = false;
        }

        pthread_mutex_unlock(&engineMutex);

        // do main tasks here
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "### mainLoop ###");

        timespec tm{
            .tv_sec = 0,
            .tv_nsec = 100000000,
        };
        nanosleep(&tm, nullptr);
    }
    return nullptr;
}

static void ANativeActivity_onStart(ANativeActivity *activity) {
    __android_log_print(ANDROID_LOG_INFO, "main", "### ANativeActivity_onStart ###");

    engine.activity = activity;
    engine.init();
    engine.bDisplayInited = false;

    pthread_mutex_init(&engineMutex, nullptr);
    pthread_mutex_init(&mainLoopShouldExit, nullptr);

    pthread_mutex_lock(&mainLoopShouldExit);
    bMainLoopThreadRunning = true;
    pthread_create(&mainLoopTID, nullptr, mainLoop, nullptr);
}

// make sure all resources are freed, all threads are joined
static void ANativeActivity_onStop(ANativeActivity *activity) {
    __android_log_print(ANDROID_LOG_INFO, "main", "### ANativeActivity_onStop ###");

    pthread_mutex_unlock(&mainLoopShouldExit);
    pthread_join(mainLoopTID, nullptr);
    bMainLoopThreadRunning = false;

    pthread_mutex_destroy(&mainLoopShouldExit);
    pthread_mutex_destroy(&engineMutex);

    engine.destroy();
    engine.activity = nullptr;
}

static void ANativeActivity_onNativeWindowCreated(ANativeActivity *activity,
        ANativeWindow *window) {
    __android_log_print(ANDROID_LOG_INFO, "main",
            "### ANativeActivity_onNativeWindowCreated ###");

    if(bMainLoopThreadRunning) {
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

    if(bMainLoopThreadRunning) {
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
}

static void ANativeActivity_onPause(ANativeActivity* activity) {
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "### ANativeActivity_onPause ###");
}

JNIEXPORT
void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState,
                              size_t savedStateSize) {
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "### ANativeActivity_onCreate ###");

    activity->callbacks->onStart = ANativeActivity_onStart;
    activity->callbacks->onStop = ANativeActivity_onStop;
    activity->callbacks->onNativeWindowCreated = ANativeActivity_onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = ANativeActivity_onNativeWindowDestroyed;
    activity->callbacks->onResume = ANativeActivity_onResume;
    activity->callbacks->onPause = ANativeActivity_onPause;
}
