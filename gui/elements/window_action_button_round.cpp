/**
 * @file        window_action_button_round.cpp
 * @brief       Round Action Button (part of #x86 GUI Framework)
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <gui/elements/window_action_button_round.h>

ACRButton::ACRButton(Widget* parent, int32_t x, int32_t y, const char* label)
    : ACButton(parent, x, y, label) {
    // Set specific font for window controls
    this->font = FontManager::activeInstance->getNewFont();
    this->font->setSize(SMALL);

    // Calculate Square/Circle dimensions
    int32_t textW = font->getStringLength(label);
    int32_t textH = font->getLineHeight();

    // Make it a square box that fits the text
    int32_t diameter = (textW > textH) ? textW : textH;
    diameter += 4;  // Padding

    this->w = diameter;
    this->h = diameter;

    // Allocate proper cache immediately
    if (cache) delete[] cache;
    cache = new uint32_t[w * h]();
}

ACRButton::~ACRButton() {}

void ACRButton::RedrawToCache() {
    // Clear Background (Transparent)
    memset(cache, 0, sizeof(uint32_t) * w * h);

    int32_t radius = w / 2;

    // 1. Background Fill
    uint32_t bgColor = isPressed ? WINDOW_CLOSE_BUTTON_BACKGROUND_COLOR_PRESSED
                                 : WINDOW_CLOSE_BUTTON_BACKGROUND_COLOR_NORMAL;

    NINA::activeInstance->FillCircle(cache, w, h, radius, radius, radius, bgColor);

    // 2. Border
    uint32_t borderColor = isPressed ? WINDOW_CLOSE_BUTTON_BORDER_COLOR_PRESSED
                                     : WINDOW_CLOSE_BUTTON_BORDER_COLOR_NORMAL;

    NINA::activeInstance->DrawCircle(cache, w, h, radius, radius, radius, borderColor);

    // 3. Centered Text
    int32_t textW = font->getStringLength(label);
    int32_t textH = font->getLineHeight();

    int32_t textX = (w - textW) / 2;
    int32_t textY = (h - textH) / 2;

    uint32_t textColor = isPressed ? BUTTON_TEXT_COLOR_PRESSED : BUTTON_TEXT_COLOR_NORMAL;

    NINA::activeInstance->DrawString(cache, w, h, textX, textY - 2, label, this->font, textColor);

    isDirty = false;
}
