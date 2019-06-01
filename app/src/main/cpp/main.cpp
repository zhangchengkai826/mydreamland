//
// Created by andys on 5/10/2019.
//

#include <engine.h>


void android_main(android_app *app)
{
    struct engine engine;

    app->userData = &engine;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    engine.app = app;

    if(app->savedState != nullptr) {
        engine.state = *(struct saved_state *)app->savedState;
    }

    while(true) {
        int events;
        struct android_poll_source *source;

        while(ALooper_pollAll(engine.animating ? 0 : -1, nullptr, &events,
                                       (void **)&source) >= 0) {
            if(source != nullptr) {
                source->process(app, source);
            }

            if(app->destroyRequested != 0) {
                return;
            }
        }

        if(engine.animating) {
            engine_draw_frame(&engine);
            engine.animating = 0;
        }
    }
}