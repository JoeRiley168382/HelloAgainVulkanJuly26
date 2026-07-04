#ifndef _BEE_TRIANGLE_
#define _BEE_TRIANGLE_

#include "VulkanApp/context.h"
#include "VulkanRenderer/renderer.h"
#include "VulkanRenderer/pipeline.h"

class BeeTriangleApp {
public:
    void Start();
    void RunLoop();
    void Stop();
    const uint32_t mWinWidth = 1280;
    const uint32_t mWinHeight = 720;
    VulkanApp mContext;
    VulkanRenderer mRenderer;
    VulkanPipeline mTriPipe;
    SDL_Event mCurrentEvent;
};

#endif