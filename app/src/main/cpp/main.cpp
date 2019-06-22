//
// Created by andys on 5/10/2019.
//

#include <engine.h>

static Engine engine;

static pthread_t renderLoopTID, physicsLoopTID, inputLoopTID;
static pthread_mutex_t renderLoopShouldExit, physicsLoopShouldExit, inputLoopShouldExit;

static pthread_mutex_t renderInputSync = PTHREAD_MUTEX_INITIALIZER;

static void *inputLoop(void *arg) {
    int rate = 30; /* unit: s^-1 */
    float dt = 1.0f / rate;
    long dt_nsec = static_cast<int>(dt * 1000000000);

    int x, y;

    while(true) {
        if(pthread_mutex_trylock(&inputLoopShouldExit) == 0) {
            // lock succeed
            pthread_mutex_unlock(&inputLoopShouldExit);
            break;
        };

        pthread_mutex_lock(&renderInputSync);

        // do tasks here
        if(engine.inputQueue) {
            __android_log_print(ANDROID_LOG_INFO, "main",
                                "### inputLoop ###");
            AInputEvent *event;
            if(AInputQueue_getEvent(engine.inputQueue, &event) >= 0) {

                if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
                    switch(AMotionEvent_getAction(event)) {
                        case AMOTION_EVENT_ACTION_DOWN: {
                            x = static_cast<int>(AMotionEvent_getX(event, 0));
                            y = static_cast<int>(AMotionEvent_getY(event, 0));

                            __android_log_print(ANDROID_LOG_INFO, "main",
                                                "Tap Down!!! (%d, %d)", x, y);
                            break;
                        }
                        case AMOTION_EVENT_ACTION_MOVE: {
                            x = static_cast<int>(AMotionEvent_getX(event, 0));
                            y = static_cast<int>(AMotionEvent_getY(event, 0));

                            __android_log_print(ANDROID_LOG_INFO, "main",
                                                "Tap Move!!! (%d, %d)", x, y);
                            break;
                        }
                        case AMOTION_EVENT_ACTION_UP: {
                            x = static_cast<int>(AMotionEvent_getX(event, 0));
                            y = static_cast<int>(AMotionEvent_getY(event, 0));

                            __android_log_print(ANDROID_LOG_INFO, "main",
                                                "Tap Up!!! (%d, %d)", x, y);
                            break;
                        }
                        default:
                            break;
                    }
                }

                AInputQueue_finishEvent(engine.inputQueue, event, 1);
            }
        }

        pthread_mutex_unlock(&renderInputSync);

        timespec tm{
                .tv_sec = 0,
                .tv_nsec = dt_nsec,
        };
        nanosleep(&tm, nullptr);
    }
    return nullptr;
}

static void *physicsLoop(void *arg) {
    int rate = 24; /* unit: s^-1 */
    float dt = 1.0f / rate;
    long dt_nsec = static_cast<int>(dt * 1000000000);
    int loopId = 0; /* 0 <= loopId < rate */

    while(true) {
        if(pthread_mutex_trylock(&physicsLoopShouldExit) == 0) {
            // lock succeed
            pthread_mutex_unlock(&physicsLoopShouldExit);
            break;
        };

        // do tasks here
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "### physicsLoop ###");

        pthread_mutex_lock(&engine.mutex);
        for(auto it = engine.object3ds->begin(); it != engine.object3ds->end(); it++) {
            it->second.modelMat = it->second.animController.advance(dt);

            /*__android_log_print(ANDROID_LOG_INFO, "main",
                                "physics modelMat: %s", glm::to_string(it->second.modelMat).c_str());*/
        }
        pthread_mutex_unlock(&engine.mutex);

        if(loopId == 0) {
            __android_log_print(ANDROID_LOG_INFO, "main",
                                "fps: %d", engine.fpsFrameCounter->exchange(0));
        }

        loopId++;
        if(loopId >= rate) {
            loopId = 0;
        }

        timespec tm{
                .tv_sec = 0,
                .tv_nsec = dt_nsec,
        };
        nanosleep(&tm, nullptr);
    }
    return nullptr;
}

static void *renderLoop(void *arg) {
    while(true) {
        if(pthread_mutex_trylock(&renderLoopShouldExit) == 0) {
            // lock succeed
            pthread_mutex_unlock(&renderLoopShouldExit);
            break;
        };

        // do tasks here
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "### renderLoop ###");

        if(engine.bAnimating) {
            engine.drawFrame();

            engine.fpsFrameCounter->fetch_add(1);
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
}

// make sure all resources are freed, all threads are joined
static void ANativeActivity_onStop(ANativeActivity *activity) {
    __android_log_print(ANDROID_LOG_INFO, "main", "### ANativeActivity_onStop ###");
}

/* Here we assume after each onNativeWindowCreated call there will be exactly one
 * onNativeWindowDestroyed call before the process gets killed by the os.
 *
 * Note if the process gets killed, cpp destructors may not be called, and smart pointers
 * may failed to be auto-deleted. So every thing should be explicit.
 *
 * All global variables should be only treated as public data stores for all running threads.
 * They should only use their default constructor, and their default constructor should do no-op.
 * All members of a class who instantiates global variables should only use their
 * default constructor, and their default constructor should do no-op.
 */
static void ANativeActivity_onNativeWindowCreated(ANativeActivity *activity,
        ANativeWindow *window) {
    __android_log_print(ANDROID_LOG_INFO, "main",
            "### ANativeActivity_onNativeWindowCreated ###");

    /* global initialize, only main thread exists */

    pthread_mutex_init(&engine.mutex, nullptr);
    engine.activity = activity;
    engine.window = window;
    engine.fpsFrameCounter = new std::atomic_int(0);
    engine.bAnimating = true;
    engine.init();

    /* global initialize end */

    pthread_mutex_init(&renderLoopShouldExit, nullptr);
    pthread_mutex_init(&physicsLoopShouldExit, nullptr);
    pthread_mutex_init(&inputLoopShouldExit, nullptr);

    pthread_mutex_lock(&renderLoopShouldExit);
    pthread_mutex_lock(&physicsLoopShouldExit);
    pthread_mutex_lock(&inputLoopShouldExit);

    pthread_create(&renderLoopTID, nullptr, renderLoop, nullptr);
    pthread_create(&physicsLoopTID, nullptr, physicsLoop, nullptr);
    pthread_create(&inputLoopTID, nullptr, inputLoop, nullptr);
}

/* if user **press power button when app window is showing**, this method is NOT guaranteed to be
 * called. If fortunately it is called, it MAY abort in the middle, which MAY cause crash and/or
 * resource leaks. So, DO NOT **press power button when app window is showing**.
 */
static void ANativeActivity_onNativeWindowDestroyed(ANativeActivity *activity,
        ANativeWindow *window) {
    __android_log_print(ANDROID_LOG_INFO, "main",
            "### ANativeActivity_onNativeWindowDestroyed ###");

    pthread_mutex_unlock(&renderLoopShouldExit);
    pthread_mutex_unlock(&physicsLoopShouldExit);
    pthread_mutex_unlock(&inputLoopShouldExit);

    pthread_join(renderLoopTID, nullptr);
    pthread_join(physicsLoopTID, nullptr);
    pthread_join(inputLoopTID, nullptr);

    pthread_mutex_destroy(&renderLoopShouldExit);
    pthread_mutex_destroy(&physicsLoopShouldExit);
    pthread_mutex_destroy(&inputLoopShouldExit);

    /* global destroy, only main thread exists */

    engine.destroy();
    delete engine.fpsFrameCounter;
    pthread_mutex_destroy(&engine.mutex);

    /* global destroy end */
}

static void ANativeActivity_onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "### ANativeActivity_onInputQueueCreated ###");

    pthread_mutex_destroy(&physicsLoopShouldExit);
    engine.inputQueue = queue;
    pthread_mutex_unlock(&renderInputSync);
}

static void ANativeActivity_onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "### ANativeActivity_onInputQueueDestroyed ###");

    pthread_mutex_destroy(&physicsLoopShouldExit);
    engine.inputQueue = nullptr;
    pthread_mutex_unlock(&renderInputSync);
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
            "checkPermission", "(Landroid/app/NativeActivity;)Z");
    jboolean bPermissionGranted = activity->env->CallStaticBooleanMethod(jclass_Util,
            jmethod_Util_static_checkPermission, activity->clazz);
    if(!bPermissionGranted) {
        return;
    }

    activity->callbacks->onStart = ANativeActivity_onStart;
    activity->callbacks->onStop = ANativeActivity_onStop;
    activity->callbacks->onNativeWindowCreated = ANativeActivity_onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = ANativeActivity_onNativeWindowDestroyed;
    activity->callbacks->onResume = ANativeActivity_onResume;
    activity->callbacks->onPause = ANativeActivity_onPause;
    activity->callbacks->onInputQueueCreated = ANativeActivity_onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = ANativeActivity_onInputQueueDestroyed;

    engine.inputQueue = nullptr;
}
