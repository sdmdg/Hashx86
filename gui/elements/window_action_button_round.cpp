/**
 * @file        window_action_button.cpp
 * @brief       Action Button Round for Window (part of #x86 GUI Framework)
 * 
 * @date        24/04/2025
 * @version     1.0.0-beta
 */

#include <gui/elements/window_action_button_round.h>

ACRButton::ACRButton(Widget* parent, int32_t x, int32_t y, const char* label)
    : ACButton(parent, x, y, label)
{
}

ACRButton::~ACRButton()
{
}

void ACRButton::RedrawToCache()
{
    uint8_t radius = w/2;
    //DEBUG_LOG("ACRButton %d: Updating", this->ID);
    NINA::activeInstance->FillCircle(cache, w, h, radius, radius, radius, isPressed ? WINDOW_CLOSE_BUTTON_BACKGROUND_COLOR_PRESSED : WINDOW_CLOSE_BUTTON_BACKGROUND_COLOR_NORMAL);
    NINA::activeInstance->DrawCircle(cache, w, h, radius, radius, radius, isPressed ? WINDOW_CLOSE_BUTTON_BORDER_COLOR_PRESSED : WINDOW_CLOSE_BUTTON_BORDER_COLOR_NORMAL);
    NINA::activeInstance->DrawString(cache, w, h, paddingX + 1, paddingY -2, label, this->font, isPressed ? BUTTON_TEXT_COLOR_PRESSED : BUTTON_TEXT_COLOR_NORMAL);
    
    isDirty = false;
}

