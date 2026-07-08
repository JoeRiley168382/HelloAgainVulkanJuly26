// vulkancontext.h
#ifndef _VULKAN_APP_
#define _VULKAN_APP_

#include "Context/context.h"
#include "RenderFrame/renderer.h"

class App {
public:
    bool Start(int aW, int aH);
    void RunLoop();
    bool HandleResize();
    Context mContext;
    Renderer mRenderer;
    SDL_Event mCurrentEvent;
protected:
    virtual void SetupPipelines() {}
    virtual void SetupRenderObjects() {}
    //Called once per frame, before DrawFrame, with the frame-in-flight index
    //that's about to be recorded/presented. Write any per-frame uniform data here.
    virtual void UpdateUniforms(uint32_t aFrameInd) {}
    virtual bool DrawFrame() = 0;
    virtual ~App() = default;
};

#endif