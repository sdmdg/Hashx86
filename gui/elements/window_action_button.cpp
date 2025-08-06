/**
 * @file        window_action_button.cpp
 * @brief       Action Button for Window (part of #x86 GUI Framework)
 * 
 * @date        24/04/2025
 * @version     1.0.0-beta
 */

#include <gui/elements/window_action_button.h>

ACButton::ACButton(Widget* parent, int32_t x, int32_t y, const char* label)
    : Button(parent, x, y,
             2 * 4 + (new SegoeUI())->getStringLength(label),
             2 * 4 + (new SegoeUI())->chrHeight,
             label)
{
    this->paddingX = 4;
    this->paddingY = 4;
}

ACButton::~ACButton()
{
}

void ACButton::OnClick(void (*callback)()) {
    onClickPtr = callback;
}

void ACButton::OnClick(void* instance, void (*callback)(void*)) {
    callbackInstance = instance;
    onClickMemberPtr = callback;
}

void ACButton::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
    if (isPressed & isVisible) {
        isPressed = false;
        update();
        if (onClickPtr) {
            onClickPtr();
        } else if (onClickMemberPtr && callbackInstance) {
            onClickMemberPtr(callbackInstance);  // Call member function via instance
        }
        // DEBUG_LOG("Event added with %d, %d, addres 0x%x", new_event->widgetID, new_event->eventType, new_event);
    }
}
