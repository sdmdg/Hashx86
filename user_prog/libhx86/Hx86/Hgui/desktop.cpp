/**
 * @file        Desktop.cpp
 * @brief       Desktop (part of #x86 GUI Framework)
 *
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <Hx86/Hgui/desktop.h>
#include <Hx86/Hx86.h>

Desktop* Desktop::activeInstance = nullptr;
static volatile uint32_t g_focusedWidgetID = 0;

static void DispatchKeyToFocused(uint8_t scancode, bool shiftPressed) {
    if (!Desktop::activeInstance || g_focusedWidgetID == 0) return;

    Widget* target = Desktop::activeInstance->FindWidgetByID((uint32_t)g_focusedWidgetID);
    if (!target) {
        g_focusedWidgetID = 0;
        return;
    }

    if (target->onKeyPressPtr) {
        target->onKeyPressPtr(scancode, shiftPressed);
    } else if (target->onKeyPressMemberPtr && target->keyCallbackInstance) {
        target->onKeyPressMemberPtr(target->keyCallbackInstance, scancode, shiftPressed);
    }
}

Desktop::Desktop() : CompositeWidget(0, 0, 0, 1, 1) {
    activeInstance = this;
    this->ID = 0;
    this->innitEventHandler();
}

Desktop::~Desktop() {}

void EventHandlerHGUI(void* arg) {
    (void)arg;
    printf("Event Handler thread started\n");

    uint8_t prevKeys[128];
    for (int i = 0; i < 128; i++) {
        prevKeys[i] = 0;
    }

    while (1) {
        if (!Desktop::activeInstance) {
            syscall_sleep(1);
            continue;
        }

        Widget* tmpWidget = nullptr;
        int32_t ret = (int32_t)HguiAPI(EVENT, GET, nullptr);

        if (ret >= 0) {
            uint32_t widgetID = (ret >> 16);
            uint32_t event = (EVENT_TYPE)ret & 0xFFFF;
            // printf("Widget Id : %d, Event Id : %d\n", widgetID, event);

            switch (event) {
                case ON_WINDOW_CLOSE:
                    tmpWidget = Desktop::activeInstance->FindWidgetByID(widgetID);
                    if (tmpWidget) {
                        if (g_focusedWidgetID == widgetID) {
                            g_focusedWidgetID = 0;
                        }

                        if (!tmpWidget->parent || tmpWidget->parent->ID == 0) {
                            syscall_exit(10);
                        } else {
                            tmpWidget->parent->RemoveChild(tmpWidget);

                            // Also remove kernel-side widget record if present.
                            WidgetData deleteData = {0, (int32_t)tmpWidget->ID};
                            HguiAPI(WIDGET, DELETE, (void*)&deleteData);
                        }
                    }

                    break;

                case ON_CLICK:
                    tmpWidget = Desktop::activeInstance->FindWidgetByID(widgetID);
                    if (tmpWidget) {
                        g_focusedWidgetID = widgetID;

                        if (tmpWidget->onClickPtr) {
                            tmpWidget->onClickPtr();  // Call non-member function
                        } else if (tmpWidget->onClickMemberPtr && tmpWidget->callbackInstance) {
                            tmpWidget->onClickMemberPtr(
                                tmpWidget->callbackInstance);  // Call member function via instance
                        }
                    }

                    break;

                default:
                    break;
            }
        }

        InputState input;
        syscall_get_input(&input);

        bool shiftPressed = input.keyStates[0x2A] || input.keyStates[0x36];

        for (uint8_t sc = 0; sc < 128; sc++) {
            if (input.keyStates[sc] && !prevKeys[sc]) {
                DispatchKeyToFocused(sc, shiftPressed);
            }
        }

        for (int i = 0; i < 128; i++) {
            prevKeys[i] = input.keyStates[i];
        }

        syscall_sleep(8);
    }
}

void Desktop::innitEventHandler() {
    uint32_t eventTid = syscall_register_event_handler(EventHandlerHGUI, nullptr);
    printf("[PROG] : GUI event handler thread TID : %d\n", eventTid);
}
