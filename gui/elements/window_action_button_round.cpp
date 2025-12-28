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
    this->font = FontManager::activeInstance->getNewFont();
    this->font->setSize(SMALL);
    this->w = font->getStringLength(label);
    this->h = font->getLineHeight();
}

ACRButton::~ACRButton()
{
}

void ACRButton::RedrawToCache()
{
    // Determine circle size (fit text + some padding)
    int32_t textW = font->getStringLength(label);
    int32_t textH = font->getLineHeight();
    
    int32_t diameter = textW;
    if (textH >= textW)
        diameter = textH;

    // Ensure cache matches new size
    if (w != diameter || h != diameter) {
        w = h = diameter;
        delete[] cache;
        cache = new uint32_t[w * h];
    }

    int32_t radius = diameter / 2;

    // Background fill
    NINA::activeInstance->FillCircle(
        cache, w, h,
        radius, radius, radius,
        isPressed ? WINDOW_CLOSE_BUTTON_BACKGROUND_COLOR_PRESSED
                  : WINDOW_CLOSE_BUTTON_BACKGROUND_COLOR_NORMAL
    );

    // Border
    NINA::activeInstance->DrawCircle(
        cache, w, h,
        radius, radius, radius,
        isPressed ? WINDOW_CLOSE_BUTTON_BORDER_COLOR_PRESSED
                  : WINDOW_CLOSE_BUTTON_BORDER_COLOR_NORMAL
    );

    // Center text
    int32_t textX = (w - textW) / 2;
    int32_t textY = (h - textH) / 2 - 2;
    NINA::activeInstance->DrawString(
        cache, w, h,
        textX, textY,
        label, this->font,
        isPressed ? BUTTON_TEXT_COLOR_PRESSED
                  : BUTTON_TEXT_COLOR_NORMAL
    );

    isDirty = false;
}


