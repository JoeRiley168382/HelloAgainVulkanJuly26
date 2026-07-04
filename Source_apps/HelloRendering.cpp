
#include "stub.h"

int main() 
{
    StubApp app{};
    app.Start();
    app.RunLoop();
    app.Stop();
    return 0;
}