/**
 * @file        window_action_button.cpp
 * @brief       Action Button (part of #x86 GUI Framework)
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <gui/elements/window_action_button.h>

ACButton::ACButton(Widget* parent, int32_t x, int32_t y, const char* label)
    : Button(parent, x, y, 0, 0, label)  // Size 0 initially, will be set by subclass or SetWidth
{
    this->onClickPtr = nullptr;
    this->onClickMemberPtr = nullptr;
    this->callbackInstance = nullptr;
}

ACButton::~ACButton() {}

void ACButton::OnClick(void (*callback)()) {
    onClickPtr = callback;
}

void ACButton::OnClick(void* instance, void (*callback)(void*)) {
    callbackInstance = instance;
    onClickMemberPtr = callback;
}

void ACButton::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    // Note: Button::OnMouseUp handles the "isPressed" state change
    if (isPressed && isVisible) {
        // Reset state
        isPressed = false;
        MarkDirty();

        // Fire Callbacks
        if (onClickPtr) {
            onClickPtr();
        } else if (onClickMemberPtr && callbackInstance) {
            onClickMemberPtr(callbackInstance);
        }
    }
}
