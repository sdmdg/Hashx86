#include <Hx86/Hx86.h>
#include <Hx86/debug.h>

void init_sys(void* arg)
{
    if(!args) {
        args = (ProgramArguments*)arg;
        heapData = syscall_heap();
        heap_init((void*)heapData.param0, (void*)heapData.param1);
    }
}