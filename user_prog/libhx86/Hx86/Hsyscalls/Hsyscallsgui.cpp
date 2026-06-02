/**
 * @file        Hsyscallsgui.cpp
 * @brief       Hx86 GUI System Call API
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <Hx86/Hsyscalls/Hsyscallsgui.h>
#include <Hx86/Hsyscalls/syscalls.h>

uint32_t HguiAPI(REQ_Element element, REQ_MODE mode, void* data) {
    return syscall_Hgui(element, mode, data);
};
