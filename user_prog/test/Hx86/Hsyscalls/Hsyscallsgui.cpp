#include <Hx86/Hsyscalls/Hsyscallsgui.h>


uint32_t HguiAPI(REQ_Element element, REQ_MODE mode, void* data){
    return syscall_Hgui(element, mode, data);
};
