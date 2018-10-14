#include "serverProcess.h"
#include "trace.h"

int main()
{
    ServerProcess serverProcess;
    TRACE(2, "starting server");
    serverProcess.run();
}
