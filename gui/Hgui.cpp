
#include <gui/Hgui.h>

HguiHandler* HguiHandler::activeInstance = nullptr;

HguiHandler::HguiHandler(uint8_t InterruptNumber, InterruptManager* interruptManager)
:    InterruptHandler(InterruptNumber + 0x20, interruptManager)
{
    this->activeInstance = this;
    HguiWidgets.Add(Desktop::activeInstance);
}

HguiHandler::~HguiHandler()
{
}

uint32_t HguiHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;

    switch ((uint32_t)cpu->eax)
    {
    case WIDGET:
        return HandleWidget(esp);
        break;
    case WINDOW:
        return HandleWindow(esp);
        break;
    case BUTTON:
        return HandleButton(esp);
        break;
    case LABEL:
        return HandleLabel(esp);
        break;
    case EVENT:
        return HandleEvent(esp);
        break;
    
    default:
        break;
    }

    return esp;
}

uint32_t HguiHandler::HandleWidget(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;
    int32_t* return_data = (int32_t*)cpu->edx;
    if (!return_data) return esp;

    if ((uint32_t)cpu->ebx == ADD_CHILD)
    {
        CompositeWidget* parentWidget = (CompositeWidget*) this->FindWidgetByID(_data->param0);
        if (!parentWidget) {
            *return_data = -1;
            return esp;
        }

        CompositeWidget* childWidget = (CompositeWidget*) this->FindWidgetByID(_data->param1);
        if (!parentWidget) {
            *return_data = -1;
            return esp;
        }

        // TODO: Add checks for widgets before add to desktop
        parentWidget->AddChild(childWidget);

        *return_data = 1;
        return esp;
    }
    else if ((uint32_t)cpu->ebx == DELETE) 
    {
        HguiWidgets.Remove([&](Widget* c) {
            return c->ID == _data->param1;
        });

        // TODO: Implement cleanup
        *return_data = 1;
        return esp;
    }

    return esp;
}

uint32_t HguiHandler::HandleWindow(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;
    int32_t* return_data = (int32_t*)cpu->edx;
    if (!return_data) return esp;

    if ((uint32_t)cpu->ebx == CREATE) 
    {
        CompositeWidget* parentWidget = (CompositeWidget*) this->FindWidgetByID(_data->param0);
        if (!parentWidget) {
            *return_data = -1;
            return esp;
        }

        uint32_t _newID = this->getNewID();
        Widget* _widget = new Window(parentWidget, _data->param1, _data->param2, _data->param3, _data->param4);
        _widget->SetPID(ProcessManager::activeInstance->getCurrentPID());
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        *return_data = _newID;
        return esp;
    } 
    else if ((uint32_t)cpu->ebx == SET_TEXT) 
    {
        Window* widget = (Window*)this->FindWidgetByID(_data->param0);
        if (!widget) {
            *return_data = -1;
            return esp;
        }

        widget->setWindowTitle(_data->param5);

        *return_data = -1;
        return esp;
    }

    return esp;
}

uint32_t HguiHandler::HandleButton(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;
    int32_t* return_data = (int32_t*)cpu->edx;
    if (!return_data) return esp;

    if ((uint32_t)cpu->ebx == CREATE) 
    {
        CompositeWidget* parentWidget = (CompositeWidget*) this->FindWidgetByID(_data->param0);
        if (!parentWidget | parentWidget->ID == 0) {
            *return_data = -1;
            return esp;
        }

        uint32_t _newID = this->getNewID();
        Widget* _widget = new Button(parentWidget, _data->param1, _data->param2, _data->param3, _data->param4, _data->param5);
        _widget->SetPID(ProcessManager::activeInstance->getCurrentPID());
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        *return_data = _newID;
        return esp;
    }

    return esp;
}

uint32_t HguiHandler::HandleLabel(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    WidgetData* _data = (WidgetData*)cpu->ecx;
    int32_t* return_data = (int32_t*)cpu->edx;
    if (!return_data) return esp;

    if ((uint32_t)cpu->ebx == CREATE) 
    {
        CompositeWidget* parentWidget = (CompositeWidget*) this->FindWidgetByID(_data->param0);
        if (!parentWidget || parentWidget->ID == 0) {
            *return_data = -1;
            return esp;
        }

        uint32_t _newID = this->getNewID();
        Widget* _widget = new Label(parentWidget, _data->param1, _data->param2, _data->param3, _data->param4, _data->param5);
        _widget->SetPID(ProcessManager::activeInstance->getCurrentPID());
        _widget->SetID(_newID);

        HguiWidgets.Add(_widget);
        *return_data = _newID;
        return esp;
    }
    else if ((uint32_t)cpu->ebx == SET_TEXT) 
    {
        Label* widget = (Label*)this->FindWidgetByID(_data->param0);
        if (!widget) {
            *return_data = -1;
            return esp;
        }

        widget->setText(_data->param5);

        *return_data = -1;
        return esp;
    }
    else if ((uint32_t)cpu->ebx == SET_FONT_SIZE) 
    {
        Label* widget = (Label*)this->FindWidgetByID(_data->param0);
        if (!widget) {
            *return_data = -1;
            return esp;
        }

        widget->setSize((FontSize)_data->param1);

        *return_data = -1;
        return esp;
    }

    return esp;
}

uint32_t HguiHandler::HandleEvent(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    int32_t* return_data = (int32_t*)cpu->edx;
    if (!return_data) return esp;

    if ((uint32_t)cpu->ebx == GET) 
    {
        if((uint32_t)ProcessManager::activeInstance->getCurrentProcess()->eventQueue.GetSize() > 0){
            Event* tmp = ProcessManager::activeInstance->getCurrentProcess()->eventQueue.PopFront();
            *return_data = (tmp->widgetID << 16) | tmp->eventType;
        } else {
            *return_data = -1;
        }
        return esp;
    }

    return esp;
}

Widget* HguiHandler::FindWidgetByID(uint32_t searchID)
{
    Widget* result = nullptr;
    HguiWidgets.ForEach([&](Widget* c) {
        if (c->ID == searchID)
            result = c;
    });
    
    return result;
}

void HguiHandler::RemoveAppByPID(uint32_t PID)
{
    HguiWidgets.Remove([&](Widget* c) {
        return c->PID == PID;
    });
}

uint32_t HguiHandler::getNewID()
{
    return widgetIDCounter++;
}

