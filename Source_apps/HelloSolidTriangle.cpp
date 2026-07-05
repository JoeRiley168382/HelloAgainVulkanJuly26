
#include "BeeTriangleStub/triangleStubApp0.h"

int main() 
{
    BeeTriangleApp app{};
    app.Start(app.mWinWidth, app.mWinHeight);
    app.RunLoop();
    return 0;
}