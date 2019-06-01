//
// Created by andys on 5/10/2019.
//

#include <engine.h>


void android_main(android_app *app)
{
    app->userData = &engine;
    app->onAppCmd = Engine::cmdHandler;
    app->onInputEvent = Engine::inputHandler;
    engine.app = app;

    if(app->savedState != nullptr) {
        engine.state = *reinterpret_cast<Engine::SavedState *>(app->savedState);
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
            engine.drawFrame();
            engine.animating = false;
        }
    }
}