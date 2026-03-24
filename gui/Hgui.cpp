/**
 * @file        Hgui.cpp
 * @brief       Hgui Handler (part of #x86 GUI Framework)
 *
 * @date        11/02/2026
 * @version     1.0.0
 */

#define KDBG_COMPONENT "GUI"
#include <gui/Hgui.h>

HguiHandler* HguiHandler::activeInstance = nullptr;

HguiHandler::HguiHandler(uint8_t InterruptNumber, InterruptManager* interruptManager)
    : InterruptHandler(InterruptNumber + 0x20, interruptManager) {
    this->activeInstance = this;

    if (Desktop::activeInstance) {
        HguiWidgets.Add(Desktop::activeInstance);
    }
}

HguiHandler::~HguiHandler() {}

uint32_t HguiHandler::HandleInterrupt(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    int32_t ret = -1;

    switch ((uint32_t)cpu->eax) {
        case WIDGET:
            ret = HandleWidget(esp);
            break;
        case WINDOW:
            ret = HandleWindow(esp);
            break;
        case BUTTON:
            ret = HandleButton(esp);
            break;
        case LABEL:
            ret = HandleLabel(esp);
            break;
        case LISTVIEW:
            ret = HandleListView(esp);
            break;
        case TERMINAL_VIEW:
            ret = HandleTerminalView(esp);
            break;
        case EVENT:
            ret = HandleEvent(esp);
            break;

        default:
            break;
    }

    cpu->eax = (uint32_t)ret;
    return esp;
}

int32_t HguiHandler::HandleWidget(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;

    if ((uint32_t)cpu->ebx == ADD_CHILD) {
        CompositeWidget* parentWidget = (CompositeWidget*)this->FindWidgetByID(_data->param0);
        if (!parentWidget) return -1;

        CompositeWidget* childWidget = (CompositeWidget*)this->FindWidgetByID(_data->param1);
        if (!childWidget) return -1;  // Also fixes a bug where parentWidget was checked twice

        // TODO: Add checks for widgets before add to desktop
        parentWidget->AddChild(childWidget);

        // If adding a window to the Desktop, create a taskbar tab
        if (parentWidget->ID == 0) {
            Desktop* desktop = Desktop::activeInstance;
            if (desktop && desktop->GetTaskbar()) {
                // Get the window title using the public accessor
                const char* tabTitle = "App";
                Window* win = (Window*)childWidget;
                if (win && win->getWindowTitle()) {
                    tabTitle = win->getWindowTitle();
                }
                desktop->GetTaskbar()->AddTab(childWidget->PID, tabTitle, childWidget);
            }
        }

        return 1;
    } else if ((uint32_t)cpu->ebx == DELETE) {
        HguiWidgets.Remove([&](Widget* c) { return c->ID == _data->param1; });

        // TODO: Implement cleanup
        return 1;
    }

    return -1;
}

int32_t HguiHandler::HandleWindow(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;

    if ((uint32_t)cpu->ebx == CREATE) {
        CompositeWidget* parentWidget = (CompositeWidget*)this->FindWidgetByID(_data->param0);
        if (!parentWidget) return -1;

        uint32_t _newID = this->getNewID();
        Widget* _widget =
            new Window(parentWidget, _data->param1, _data->param2, _data->param3, _data->param4);
        if (!_widget) {
            HALT("CRITICAL: Failed to allocate Window widget!\n");
        }
        _widget->SetPID(Scheduler::activeInstance->GetCurrentProcess()->pid);
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        return (uint32_t)_newID;
    } else if ((uint32_t)cpu->ebx == SET_TEXT) {
        Window* widget = (Window*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        widget->setWindowTitle(_data->param5);

        return 1;
    }

    return -1;
}

int32_t HguiHandler::HandleButton(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;

    if ((uint32_t)cpu->ebx == CREATE) {
        CompositeWidget* parentWidget = (CompositeWidget*)this->FindWidgetByID(_data->param0);
        if (!parentWidget || parentWidget->ID == 0)
            return -1;  // Minor structural fix for bitwise vs Logical or originally

        uint32_t _newID = this->getNewID();
        Widget* _widget = new Button(parentWidget, _data->param1, _data->param2, _data->param3,
                                     _data->param4, _data->param5);
        if (!_widget) {
            HALT("CRITICAL: Failed to allocate Button widget!\n");
        }
        _widget->SetPID(Scheduler::activeInstance->GetCurrentProcess()->pid);
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        return (int32_t)_newID;
    }

    return -1;
}

int32_t HguiHandler::HandleLabel(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;

    if ((uint32_t)cpu->ebx == CREATE) {
        CompositeWidget* parentWidget = (CompositeWidget*)this->FindWidgetByID(_data->param0);
        if (!parentWidget || parentWidget->ID == 0) return -1;

        uint32_t _newID = this->getNewID();
        Widget* _widget = new Label(parentWidget, _data->param1, _data->param2, _data->param3,
                                    _data->param4, _data->param5);
        if (!_widget) {
            HALT("CRITICAL: Failed to allocate Label widget!\n");
        }
        _widget->SetPID(Scheduler::activeInstance->GetCurrentProcess()->pid);
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        return (int32_t)_newID;
    } else if ((uint32_t)cpu->ebx == SET_TEXT) {
        Label* widget = (Label*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        widget->setText(_data->param5);

        return 1;
    } else if ((uint32_t)cpu->ebx == SET_FONT_SIZE) {
        Label* widget = (Label*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        widget->setSize((FontSize)_data->param1);

        return 1;
    }

    return -1;
}

int32_t HguiHandler::HandleListView(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;

    if ((uint32_t)cpu->ebx == CREATE) {
        CompositeWidget* parentWidget = (CompositeWidget*)this->FindWidgetByID(_data->param0);
        if (!parentWidget || parentWidget->ID == 0) return -1;

        uint32_t _newID = this->getNewID();
        Widget* _widget =
            new ListView(parentWidget, _data->param1, _data->param2, _data->param3, _data->param4);
        if (!_widget) {
            HALT("CRITICAL: Failed to allocate ListView widget!\n");
        }
        _widget->SetPID(Scheduler::activeInstance->GetCurrentProcess()->pid);
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        return (int32_t)_newID;
    } else if ((uint32_t)cpu->ebx == SET_ITEMS) {
        // param0 = widgetID, param5 = pointer to ListViewItemData array, param1 = count
        ListView* widget = (ListView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        struct ListViewItemData {
            char name[64];
            uint32_t size;
            uint8_t type;
        };

        widget->Clear();
        ListViewItemData* items = (ListViewItemData*)_data->param5;
        int count = (int)_data->param1;
        for (int i = 0; i < count && i < LISTVIEW_MAX_ITEMS; i++) {
            widget->AddItem(items[i].name, items[i].size, items[i].type);
        }
        return count;
    } else if ((uint32_t)cpu->ebx == CLEAR_ITEMS) {
        ListView* widget = (ListView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;
        widget->Clear();
        return 1;
    } else if ((uint32_t)cpu->ebx == GET_SELECTED) {
        ListView* widget = (ListView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;
        return widget->GetSelectedIndex();
    } else if ((uint32_t)cpu->ebx == SET_TEXT) {
        ListView* widget = (ListView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;
        widget->SetHeader(_data->param5);
        return 1;
    }

    return -1;
}

int32_t HguiHandler::HandleTerminalView(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;

    if ((uint32_t)cpu->ebx == CREATE) {
        CompositeWidget* parentWidget = (CompositeWidget*)this->FindWidgetByID(_data->param0);
        if (!parentWidget || parentWidget->ID == 0) return -1;

        uint32_t _newID = this->getNewID();
        Widget* _widget =
            new TerminalView(parentWidget, (int32_t)_data->param1, (int32_t)_data->param2,
                             (int32_t)_data->param3, (int32_t)_data->param4, _data->param5);
        if (!_widget) {
            HALT("CRITICAL: Failed to allocate TerminalView widget!\n");
        }
        _widget->SetPID(Scheduler::activeInstance->GetCurrentProcess()->pid);
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        return (int32_t)_newID;
    } else if ((uint32_t)cpu->ebx == SET_TEXT) {
        TerminalView* widget = (TerminalView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        widget->setText(_data->param5);
        return 1;
    } else if ((uint32_t)cpu->ebx == SET_FONT_SIZE) {
        TerminalView* widget = (TerminalView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        widget->setSize((FontSize)_data->param1);
        return 1;
    } else if ((uint32_t)cpu->ebx == SET_SCROLL_META) {
        TerminalView* widget = (TerminalView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        widget->setScrollMeta((int)_data->param1, (int)_data->param2, (int)_data->param3);
        return 1;
    } else if ((uint32_t)cpu->ebx == GET_SCROLL_ACTION) {
        TerminalView* widget = (TerminalView*)this->FindWidgetByID(_data->param0);
        if (!widget) return -1;

        return widget->consumeScrollAction();
    }

    return -1;
}

int32_t HguiHandler::HandleEvent(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;

    if (!Scheduler::activeInstance || !Desktop::activeInstance) {
        return -1;
    }

    ProcessControlBlock* p = Scheduler::activeInstance->GetCurrentProcess();
    if (!p) {
        return -1;
    }

    EventHandler* process_eventHandler = Desktop::activeInstance->getHandler(p->pid);
    if (!process_eventHandler) {
        return -1;
    }

    if ((uint32_t)cpu->ebx == GET) {
        if ((uint32_t)process_eventHandler->eventQueue.GetSize() > 0) {
            Event* tmp = process_eventHandler->eventQueue.PopFront();
            if (!tmp) {
                return -1;
            }

            int32_t encoded = (int32_t)((tmp->widgetID << 16) | tmp->eventType);
            delete tmp;
            return encoded;
        } else {
            // No pending events; return immediately so user-space can poll other inputs.
            return -1;
        }
    }

    return -1;
}

Widget* HguiHandler::FindWidgetByID(uint32_t searchID) {
    Widget* result = nullptr;
    HguiWidgets.ForEach([&](Widget* c) {
        if (c->ID == searchID) result = c;
    });

    return result;
}

void HguiHandler::RemoveAppByPID(uint32_t PID) {
    HguiWidgets.Remove([&](Widget* c) { return c->PID == PID; });

    if (Desktop::activeInstance) {
        Desktop::activeInstance->deleteEventHandler(PID);
    }
}

uint32_t HguiHandler::getNewID() {
    return widgetIDCounter++;
}
