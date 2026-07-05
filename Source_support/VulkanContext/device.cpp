#include "device.h"


VulkanDevice::~VulkanDevice()
{
    vkDestroyDevice(mLogicalDevice, nullptr);
}

bool VulkanDevice::Setup(VkInstance aInst, VkSurfaceKHR aSurface)
{
    //Use instance to select physical device
    unsigned int numPhysicalDevices = 0;
    vkEnumeratePhysicalDevices(aInst, &numPhysicalDevices,nullptr);
    std::vector<VkPhysicalDevice> physicalDeviceList(numPhysicalDevices, VK_NULL_HANDLE);
    vkEnumeratePhysicalDevices(aInst, &numPhysicalDevices, physicalDeviceList.data());
    //Just get the 1st (D)GPU with vulkan 1.4
    for(VkPhysicalDevice pd : physicalDeviceList)
    {
        VkPhysicalDeviceProperties pdProps;
        vkGetPhysicalDeviceProperties(pd, &pdProps);
        // BUGFIX: condition was un-negated, so it skipped (continue) devices that DO
        // support 1.4 and picked ones that don't, backwards from the "get a 1.4 GPU" intent.
        if(!(VK_API_VERSION_MAJOR(pdProps.apiVersion) == 1 && VK_API_VERSION_MINOR(pdProps.apiVersion) >= 4))
            continue;
        mPhysicalDevice = pd;
        if(pdProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)  
            break;
    }
    if(mPhysicalDevice == VK_NULL_HANDLE)
        return false;

    
    //Setup queues for logical device
    //Find queue family
    unsigned int numQueues = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &numQueues, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(numQueues);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &numQueues, queueFamilies.data());

    for(int i = 0; i < numQueues; i++) {
        if(VK_QUEUE_GRAPHICS_BIT & queueFamilies[i].queueFlags)
            mGraphicsQueueFamilyIndex = i;
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, aSurface, &presentSupport);
        if(presentSupport != VK_FALSE)
            mPresentQueueFamilyIndex = i;
    }
    if(mGraphicsQueueFamilyIndex == -1 || mPresentQueueFamilyIndex == -1)
        return false;


    //Create Logical Device
    float queuePriorities[1] = { 1.f };
	VkDeviceQueueCreateInfo queueInfos[2]{};
	queueInfos[0].sType           = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfos[0].queueFamilyIndex = mGraphicsQueueFamilyIndex;
	queueInfos[0].queueCount      = 1;
	queueInfos[0].pQueuePriorities = queuePriorities;
    uint32_t queueInfoCount = 1;
    if(mPresentQueueFamilyIndex != mGraphicsQueueFamilyIndex) {
        queueInfos[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfos[1].queueFamilyIndex = mPresentQueueFamilyIndex;
        queueInfos[1].queueCount       = 1;
        queueInfos[1].pQueuePriorities = queuePriorities;
        queueInfoCount = 2;
    }
	VkPhysicalDeviceFeatures deviceFeatures{};

	VkPhysicalDeviceVulkan13Features vk13{};
	vk13.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vk13.synchronization2  = VK_TRUE;
	vk13.dynamicRendering  = VK_TRUE;
	VkPhysicalDeviceVulkan14Features vk14{};
	vk14.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
	vk14.pNext  = &vk13;
	vk14.maintenance5  = VK_TRUE;
    char const* deviceExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo logicalDeviceInfo{};
    logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceInfo.queueCreateInfoCount = queueInfoCount;
    logicalDeviceInfo.pQueueCreateInfos = queueInfos;
    logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;
    logicalDeviceInfo.pNext = &vk14;
    logicalDeviceInfo.enabledExtensionCount = 1;
    logicalDeviceInfo.ppEnabledExtensionNames = deviceExts;
    
    if(vkCreateDevice(mPhysicalDevice, &logicalDeviceInfo, nullptr, &mLogicalDevice) != VK_SUCCESS)
        return false;
    
    vkGetDeviceQueue(mLogicalDevice, mGraphicsQueueFamilyIndex, 0, &mGraphicsQueue);
    vkGetDeviceQueue(mLogicalDevice, mPresentQueueFamilyIndex, 0, &mPresentQueue);

    return true;
}
