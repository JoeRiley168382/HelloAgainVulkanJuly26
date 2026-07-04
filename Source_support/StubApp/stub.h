#ifndef _SIMPLE_APP_
#define _SIMPLE_APP_
#include "VulkanApp/context.h"
#include "VulkanRenderer/renderer.h"

class StubApp {
public:
    void Start();
    void RunLoop();
    void Stop();
    const uint32_t mWinWidth = 1280;
    const uint32_t mWinHeight = 720;
    VulkanApp mContext;
    VulkanRenderer mRenderer;
    SDL_Event mCurrentEvent;
};

#endif