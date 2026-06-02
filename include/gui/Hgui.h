
#ifndef HGUI_SYSCALLS_H
#define HGUI_SYSCALLS_H

#include <core/interrupts.h>
#include <core/memory.h>
#include <core/scheduler.h>
#include <debug.h>
#include <gui/eventHandler.h>
#include <gui/gui.h>
#include <gui/listview.h>
#include <types.h>
#include <utils/linkedList.h>

typedef enum {
    WIDGET = 0x0,
    WINDOW = 0x1,
    BUTTON = 0x2,
    EVENT = 0x3,
    DESKTOP = 0x4,
    LABEL = 0x5,
    LISTVIEW = 0x6,
    TERMINAL_VIEW = 0x7,
} REQ_Element;

typedef enum {
    CREATE = 0x0,
    ADD_CHILD = 0x1,
    REMOVE_CHILD = 0x2,
    DELETE = 0x3,
    GET = 0x4,
    SET_TEXT = 0x5,
    SET_FONT_SIZE = 0x6,
    SET_ITEMS = 0x7,
    CLEAR_ITEMS = 0x8,
    GET_SELECTED = 0x9,
    SET_SCROLL_META = 0xA,
    GET_SCROLL_ACTION = 0xB,
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

class HguiHandler : public InterruptHandler {
private:
    LinkedList<Widget*> HguiWidgets;
    uint32_t widgetIDCounter = 1000;

public:
    static HguiHandler* activeInstance;
    HguiHandler(uint8_t InterruptNumber, InterruptManager* interruptManager);
    ~HguiHandler();

    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual int32_t HandleWidget(uint32_t esp);
    virtual int32_t HandleWindow(uint32_t esp);
    virtual int32_t HandleButton(uint32_t esp);
    virtual int32_t HandleLabel(uint32_t esp);
    virtual int32_t HandleListView(uint32_t esp);
    virtual int32_t HandleTerminalView(uint32_t esp);
    virtual int32_t HandleEvent(uint32_t esp);
    void RemoveAppByPID(uint32_t PID);
    Widget* FindWidgetByID(uint32_t searchID);
    uint32_t getNewID();
};

#endif  // HGUI_SYSCALLS_H
