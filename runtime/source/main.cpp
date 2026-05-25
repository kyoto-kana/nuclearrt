#include "Application.h"

#if defined(__PSP__)
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

PSP_MODULE_INFO("NuclearRT", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(512);
PSP_HEAP_SIZE_KB(-2048);
#endif

int main(int argc, char *argv[])
{
    Application& app = Application::Instance();
    app.Initialize();
    app.Run();
    return 0;
}