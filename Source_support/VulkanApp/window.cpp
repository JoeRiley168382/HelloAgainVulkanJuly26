#include "window.h"

VulkanWindow::~VulkanWindow()
{
    if(hDevice != nullptr) {
        vkDeviceWaitIdle(hDevice->mLogicalDevice);
        for(VkImageView view : mSwapViewList)
            vkDestroyImageView(hDevice->mLogicalDevice, view, nullptr);
        if(mSwapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(hDevice->mLogicalDevice, mSwapchain, nullptr);
    }
    if(mSurface != VK_NULL_HANDLE && hInstance != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(hInstance, mSurface, nullptr);
    if(mWindow != nullptr) {
        SDL_DestroyWindow(mWindow);
    }
}

bool VulkanWindow::SetupWindow(VkInstance aInstance,  int aW,  int aH)
{
    if(aInstance == VK_NULL_HANDLE)
        return false;
    hInstance = aInstance;
    mWindow = SDL_CreateWindow("Big boy Vulkan on my own", aW, aH, SDL_WINDOW_VULKAN);
    if(mWindow == nullptr)
        return false;
    if(!SDL_Vulkan_CreateSurface(mWindow, hInstance, nullptr, &mSurface))
        return false;

    return true;
}

bool VulkanWindow::SetupSwapchain(VulkanDevice& aDevice)
{
    //Create the swapchain
    if(aDevice.mLogicalDevice == VK_NULL_HANDLE || aDevice.mPhysicalDevice == VK_NULL_HANDLE)
        return false;
    hDevice = &aDevice;
    unsigned int numSurfaceFormats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(hDevice->mPhysicalDevice, mSurface, &numSurfaceFormats, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(numSurfaceFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(hDevice->mPhysicalDevice, mSurface, &numSurfaceFormats, surfaceFormats.data());
    unsigned int numPresentModes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(hDevice->mPhysicalDevice, mSurface, &numPresentModes, nullptr);
    std::vector<VkPresentModeKHR> presentModesList(numPresentModes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(hDevice->mPhysicalDevice, mSurface, &numPresentModes, presentModesList.data());
    std::unordered_set<VkPresentModeKHR> presentModes{};
    for (auto &&pM : presentModesList)
        presentModes.insert(pM);

    // BUGFIX: chosenSurfaceFormat was left uninitialized if neither preferred format
    // below is found; default to the first format the surface reports as a fallback.
    VkSurfaceFormatKHR chosenSurfaceFormat = surfaceFormats[0];
    for (auto fmt : surfaceFormats)
	{
		if (VK_FORMAT_R8G8B8A8_SRGB == fmt.format && VK_COLORSPACE_SRGB_NONLINEAR_KHR == fmt.colorSpace) {
			chosenSurfaceFormat = fmt;
			break;
		}
		if (VK_FORMAT_B8G8R8A8_SRGB == fmt.format && VK_COLORSPACE_SRGB_NONLINEAR_KHR == fmt.colorSpace) {
			chosenSurfaceFormat = fmt;
			break;
		}
	}

    //TODO: pick appropriate VkPresentModeKHR
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (presentModes.count(VK_PRESENT_MODE_FIFO_RELAXED_KHR))
		presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(hDevice->mPhysicalDevice, mSurface, &surfaceCapabilities);
    unsigned int imgCount = surfaceCapabilities.minImageCount + 1;
    if(surfaceCapabilities.maxImageCount > 0 && imgCount > surfaceCapabilities.maxImageCount)
        imgCount = surfaceCapabilities.maxImageCount;
    mSwapchainExtent = surfaceCapabilities.currentExtent;
    // BUGFIX: mSwapchainFormat was never assigned, so the image views created below
    // used a default/uninitialized VkFormat instead of the format the swapchain was
    // actually created with.
    mSwapchainFormat = chosenSurfaceFormat.format;

    VkSwapchainCreateInfoKHR chainInfo{};
	chainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	chainInfo.surface = mSurface;
	chainInfo.minImageCount = imgCount;
	chainInfo.imageFormat = chosenSurfaceFormat.format;
	chainInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
	chainInfo.imageExtent = mSwapchainExtent;
	chainInfo.imageArrayLayers = 1;
	chainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	chainInfo.presentMode = presentMode;
	chainInfo.clipped = VK_TRUE;
	chainInfo.oldSwapchain = VK_NULL_HANDLE;
	chainInfo.preTransform = surfaceCapabilities.currentTransform;
	chainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if(hDevice->mGraphicsQueueFamilyIndex == hDevice->mPresentQueueFamilyIndex)
        chainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    else {
        uint32_t queueInds[] = {(uint32_t)hDevice->mGraphicsQueueFamilyIndex, (uint32_t)hDevice->mPresentQueueFamilyIndex};
        chainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        chainInfo.queueFamilyIndexCount = 2;
        chainInfo.pQueueFamilyIndices = queueInds;
    }
	vkCreateSwapchainKHR(hDevice->mLogicalDevice, &chainInfo, nullptr, &mSwapchain);

    unsigned int numImages;
    vkGetSwapchainImagesKHR(hDevice->mLogicalDevice, mSwapchain, &numImages, nullptr);
    mSwapImageList = std::vector<VkImage>(numImages);
    vkGetSwapchainImagesKHR(hDevice->mLogicalDevice, mSwapchain, &numImages, mSwapImageList.data());

    for (size_t i = 0; i < mSwapImageList.size(); i++)
    {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = mSwapImageList[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = mSwapchainFormat;
		viewInfo.components = VkComponentMapping{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		viewInfo.subresourceRange = VkImageSubresourceRange{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1,
			0, 1
		};
		VkImageView view = VK_NULL_HANDLE;
        vkCreateImageView(hDevice->mLogicalDevice, &viewInfo, nullptr, &view);
        mSwapViewList.emplace_back(view);
    }
    return true;
}
