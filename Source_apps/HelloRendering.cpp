
#include "BlockColorStub/blockColorStub.h"

int main() 
{
    BlockColorStub app{};
    app.Start(app.mWinWidth, app.mWinHeight);
    app.RunLoop();
    return 0;
}