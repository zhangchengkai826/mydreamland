//
// Created by andys on 5/10/2019.
//

#include <engine.h>


void android_main(android_app *app)
{
    app->userData = &gEngine;
    app->onAppCmd = Engine::cmdHandler;
    app->onInputEvent = Engine::inputHandler;
    gEngine.app = app;

    if(app->savedState != nullptr) {
        gEngine.state = *reinterpret_cast<Engine::SavedState *>(app->savedState);
    }

    gEngine.init();

    while(true) {
        int events;
        struct android_poll_source *source;

        while(ALooper_pollAll(gEngine.animating ? 0 : -1, nullptr, &events,
                                       (void **)&source) >= 0) {
            if(source != nullptr) {
                source->process(app, source);
            }

            if(app->destroyRequested != 0) {
                gEngine.destroy();
                return;
            }
        }

        if(gEngine.animating) {
            gEngine.drawFrame();
            gEngine.animating = false;
        }
    }
}