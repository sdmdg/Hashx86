/**
 * @file        Desktop.cpp
 * @brief       Desktop (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <Hx86/Hgui/desktop.h>

Desktop* Desktop::activeInstance = nullptr;

Desktop::Desktop(): CompositeWidget(0,0,0,1,1){
    this->ID = 0;
    this->innitEventHandler();
    activeInstance = this;
}

Desktop::~Desktop(){
}


struct PrArguments {
};

void EventHandlerHGUI(void* arg){
    PrArguments* args = (PrArguments*) arg;
    printf("Event Handler thread started\n");
    while (1) {
        Widget* tmpWidget = nullptr;
        uint32_t ret = HguiAPI(EVENT, GET, args);

        if (ret != -1) {
            uint32_t widgetID = (ret >> 16);
            uint32_t event = (EVENT_TYPE) ret & 0xFFFF;
            //printf("Widget Id : %d, Event Id : %d\n", widgetID, event);

            switch (event)
            {
            case ON_WINDOW_CLOSE:
                tmpWidget = Desktop::activeInstance->FindWidgetByID(widgetID);
                if (tmpWidget){
                    if (tmpWidget->parent->ID == 0){
                        syscall_exit(10);
                    } else {
                        syscall_Hgui(DESKTOP, REMOVE_CHILD, args);
                    }
                }
                
                break;

            case ON_CLICK:
                tmpWidget = Desktop::activeInstance->FindWidgetByID(widgetID);
                if (tmpWidget){
                    if (tmpWidget->onClickPtr) {
                        tmpWidget->onClickPtr();  // Call non-member function
                    } else if (tmpWidget->onClickMemberPtr && tmpWidget->callbackInstance) {
                        tmpWidget->onClickMemberPtr(tmpWidget->callbackInstance);  // Call member function via instance
                    }
                }
                
                break;
            
            default:
                break;
            }

        }
    }
}

void Desktop::innitEventHandler(){
    PrArguments args = {};
    uint32_t tid = syscall_clone(EventHandlerHGUI, (void*)&args);
    //printf("Event Handler thread created with TID : %d\n", tid);
}
