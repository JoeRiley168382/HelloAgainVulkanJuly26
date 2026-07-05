#ifndef _BEE_TRIANGLE_
#define _BEE_TRIANGLE_

#include "VulkanApp/app.h"
#include "VulkanRenderer/pipeline.h"

class BeeTriangleApp : public VulkanApp {
public:
    const uint32_t mWinWidth = 1280;
    const uint32_t mWinHeight = 720;
    VulkanPipeline mTriPipe;
protected:
    void SetupPipelines() override;
};

#endif
