//
// Created by andys on 5/10/2019.
//

#include <android_native_app_glue.h>
#include <cstring>
#include <android/sensor.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "main", __VA_ARGS__)

struct saved_state {
    int32_t x;
    int32_t y;
};

struct engine {
    struct android_app *app;

    ASensorManager *sensorManager;
    const ASensor *accelerometerSensor;
    ASensorEventQueue *sensorEventQueue;

    int animating;
    struct saved_state state;
};

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    auto engine = (struct engine *)app->userData;
    if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = (int32_t)AMotionEvent_getX(event, 0);
        engine->state.y = (int32_t)AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    //auto engine = (struct engine *)app->userData;
    switch(cmd){
        case APP_CMD_INIT_WINDOW:
            break;
    }
}

const char kPackageName[] = "com.mydreamland.www";

void android_main(android_app *state)
{
    struct engine engine;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    engine.sensorManager = ASensorManager_getInstanceForPackage(kPackageName);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper,
            LOOPER_ID_USER, nullptr, nullptr);

    if(state->savedState != nullptr) {
        engine.state = *(struct saved_state *)state->savedState;
    }

    while(1) {
        int ident;
        int events;
        struct android_poll_source *source;

        while((ident = ALooper_pollAll(engine.animating ? 0 : -1, nullptr, &events,
                                       (void **)&source)) >= 0) {
            if(source != nullptr) {
                source->process(state, source);
            }

            if(ident == LOOPER_ID_USER) {
                if(engine.accelerometerSensor != nullptr) {
                    ASensorEvent event;
                    while(ASensorEventQueue_getEvents(engine.sensorEventQueue,
                                                      &event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                                event.acceleration.x, event.acceleration.y, event.acceleration.z);
                    }
                }
            }

            if(state->destroyRequested != 0) {
                return;
            }
        }

        if(engine.animating) {
            // draw something
        }
    }
}