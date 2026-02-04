#ifndef HSYSCALLSGUI_H
#define HSYSCALLSGUI_H

#include <Hx86/Hsyscalls/syscalls.h>
#include <Hx86/stdint.h>

typedef enum {
    WIDGET = 0x0,
    WINDOW = 0x1,
    BUTTON = 0x2,
    EVENT = 0x3,
    DESKTOP = 0x4,
    LABEL = 0x5,
} REQ_Element;

typedef enum {
    CREATE = 0x0,
    ADD_CHILD = 0x1,
    REMOVE_CHILD = 0x2,
    DELETE = 0x3,
    GET = 0x4,
    SET_TEXT = 0x5,
    SET_FONT_SIZE = 0x6,
} REQ_MODE;

uint32_t HguiAPI(REQ_Element element, REQ_MODE mode, void* data);

#endif  // HSYSCALLSGUI_H
