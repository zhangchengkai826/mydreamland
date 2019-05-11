//
// Created by andys on 5/10/2019.
//

#include <android_native_app_glue.h>
#include <vector>
#include <android/log.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "main", __VA_ARGS__)

struct saved_state {
    int32_t x;
    int32_t y;
};

struct engine {
    struct android_app *app;

    int animating;
    struct saved_state state;

    VkInstance vkInstance;
    VkSurfaceKHR vkSurface;
    VkPhysicalDevice vkGpu;
};

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    auto engine = (struct engine *)app->userData;
    if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = (int32_t)AMotionEvent_getX(event, 0);
        engine->state.y = (int32_t)AMotionEvent_getY(event, 0);
        //LOGI("x: %d, y: %d", engine->state.x, engine->state.y);
        return 1;
    }
    return 0;
}

static int engine_init_display(struct engine *engine) {
    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .apiVersion = VK_MAKE_VERSION(1, 0, 0),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .pApplicationName = "mydreamland",
            .pEngineName = "mydreamland_engine",
    };

    // prepare necessary extensions: Vulkan on Android need these to function
    std::vector<const char *> instanceExt, deviceExt;
    instanceExt.push_back("VK_KHR_surface");
    instanceExt.push_back("VK_KHR_android_surface");
    deviceExt.push_back("VK_KHR_swapchain");

    // Create the Vulkan instance
    VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<uint32_t>(instanceExt.size()),
            .ppEnabledExtensionNames = instanceExt.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
    };
    vkCreateInstance(&instanceCreateInfo, nullptr, &engine->vkInstance);

    // if we create a surface, we need the surface extension
    VkAndroidSurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = engine->app->window};
    vkCreateAndroidSurfaceKHR(engine->vkInstance, &createInfo, nullptr,
                                      &engine->vkSurface);

    // Find one GPU to use:
    // On Android, every GPU device is equal -- supporting graphics/compute/present
    // for this sample, we use the very first GPU device found on the system
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(engine->vkInstance, &gpuCount, nullptr);
    VkPhysicalDevice tmpGpus[gpuCount];
    vkEnumeratePhysicalDevices(engine->vkInstance, &gpuCount, tmpGpus);
    engine->vkGpu = tmpGpus[0];  // Pick up the first GPU Device



    return 0;
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    auto engine = (struct engine *)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)app->savedState) = engine->state;
            app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            engine_init_display(engine);
            //engine_draw_frame(engine);
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            //engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            engine->animating = 0;
            //engine_draw_frame(engine);
            break;
        default:
            break;
    }
}

void android_main(android_app *app)
{
    struct engine engine = {nullptr};

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
            //engine_draw_frame(&engine);
        }
    }
}