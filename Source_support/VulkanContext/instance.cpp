#include "instance.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace
{
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
    {
        fprintf(stderr, "[Vulkan] %s\n", callbackData->pMessage);
        return VK_FALSE;
    }
}

VulkanInstance::~VulkanInstance()
{
    if(mDebugMsgr != VK_NULL_HANDLE)
        vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMsgr, nullptr);
    if(mInstance != VK_NULL_HANDLE)
        vkDestroyInstance(mInstance, nullptr);
}

bool VulkanInstance::Setup()
{   
    //Collect the names of the supported vulkan layers
    std::unordered_set<std::string> supportedVulkanLayers{};
    unsigned int numVulkanLayers = 0;
    vkEnumerateInstanceLayerProperties(&numVulkanLayers, nullptr);
    std::vector<VkLayerProperties> vulkanLayerList(numVulkanLayers);
    vkEnumerateInstanceLayerProperties(&numVulkanLayers, vulkanLayerList.data());
    for (VkLayerProperties const& layer : vulkanLayerList)
        supportedVulkanLayers.insert(layer.layerName);

    //Collect the names of the supported vulkan extensions
    std::unordered_set<std::string> supportedVulkanExts{};
    unsigned int numVulkanExts = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &numVulkanExts, nullptr);
    std::vector<VkExtensionProperties> vulkanExtensionList(numVulkanExts);
    vkEnumerateInstanceExtensionProperties(nullptr, &numVulkanExts, vulkanExtensionList.data());
    for (VkExtensionProperties const& ext : vulkanExtensionList)
        supportedVulkanExts.insert(ext.extensionName);

    std::vector<char const*> enabledVulkanLayers{};
    std::vector<char const*> enabledVulkanExts{};
    //Enable debug
    if(supportedVulkanLayers.count("VK_LAYER_KHRONOS_validation"))
        enabledVulkanLayers.emplace_back("VK_LAYER_KHRONOS_validation");
    if(supportedVulkanExts.count("VK_EXT_debug_utils"))
        enabledVulkanExts.emplace_back("VK_EXT_debug_utils");
    // SDL requires VK_KHR_surface + platform surface extension (e.g. VK_KHR_win32_surface)
    uint32_t sdlExtCount = 0;
    const char* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
    for(uint32_t i = 0; i < sdlExtCount; i++)
        if(supportedVulkanExts.count(sdlExts[i]))
            enabledVulkanExts.emplace_back(sdlExts[i]);

    //Prepare to create the instance

    //Setup the debugMessenger
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerInfo.pfnUserCallback = DebugMessengerCallback;
    debugMessengerInfo.pUserData = nullptr;


    //Prepare the application instance
    mVulkanAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    mVulkanAppInfo.pApplicationName = "BigBoyVulkan-June26";
    mVulkanAppInfo.applicationVersion = 2026;
    mVulkanAppInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0);

    //Create the instance
    VkInstanceCreateInfo vulkanInstanceCreateInfo{};
    vulkanInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vulkanInstanceCreateInfo.enabledLayerCount = enabledVulkanLayers.size();
    vulkanInstanceCreateInfo.enabledExtensionCount = enabledVulkanExts.size();
    vulkanInstanceCreateInfo.ppEnabledLayerNames = enabledVulkanLayers.data();
    vulkanInstanceCreateInfo.ppEnabledExtensionNames = enabledVulkanExts.data();
    vulkanInstanceCreateInfo.pApplicationInfo = &mVulkanAppInfo;

    if(supportedVulkanExts.count("VK_EXT_debug_utils") != 0)
        vulkanInstanceCreateInfo.pNext = &debugMessengerInfo;

    if(vkCreateInstance(&vulkanInstanceCreateInfo, nullptr, &mInstance) != VK_SUCCESS)
        return false;

    
    volkLoadInstance(mInstance);
    if(supportedVulkanExts.count("VK_EXT_debug_utils") != 0)
        vkCreateDebugUtilsMessengerEXT(mInstance, &debugMessengerInfo, nullptr, &mDebugMsgr);

    return true;
}
