 
#ifndef HGUI_SYSCALLS_H
#define HGUI_SYSCALLS_H

#include <types.h>
#include <core/interrupts.h>
#include <core/process.h>
#include <debug.h>
#include <gui/gui.h>
#include <core/memory.h>
#include <utils/linkedList.h>
#include <gui/eventHandler.h>

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

struct WidgetData {
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
    char* param5;
    char* param6;
    char* param7;
};

class HguiHandler : public InterruptHandler
{
private:
    LinkedList<Widget*> HguiWidgets;
    uint32_t widgetIDCounter = 1000;

public:
    static HguiHandler* activeInstance;
    HguiHandler(uint8_t InterruptNumber, InterruptManager* interruptManager);
    ~HguiHandler();
    
    virtual uint32_t HandleWidget(uint32_t esp);
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual uint32_t HandleWindow(uint32_t esp);
    virtual uint32_t HandleButton(uint32_t esp);
    virtual uint32_t HandleLabel(uint32_t esp);
    virtual uint32_t HandleEvent(uint32_t esp);
    void RemoveAppByPID(uint32_t PID);
    Widget* FindWidgetByID(uint32_t searchID);
    uint32_t getNewID();
};

#endif // HGUI_SYSCALLS_H