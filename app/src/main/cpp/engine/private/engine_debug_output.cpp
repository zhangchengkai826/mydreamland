//
// Created by andys on 6/1/2019.
//
#include <engine.h>

VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugReportCallback(
        VkDebugReportFlagsEXT msgFlags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject, size_t location,
        int32_t msgCode, const char *pLayerPrefix,
        const char *pMsg, void *pUserData)
{
    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        __android_log_print(ANDROID_LOG_ERROR,
                            "main",
                            "ERROR: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        __android_log_print(ANDROID_LOG_WARN,
                            "main",
                            "WARNING: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        __android_log_print(ANDROID_LOG_WARN,
                            "main",
                            "PERFORMANCE WARNING: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        __android_log_print(ANDROID_LOG_INFO,
                            "main", "INFO: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        __android_log_print(ANDROID_LOG_VERBOSE,
                            "main", "DEBUG: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    }

    // Returning false tells the layer not to stop when the event occurs, so
    // they see the same behavior with and without validation layers enabled.
    return VK_FALSE;
}