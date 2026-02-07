/**
 * @file        button.cpp
 * @brief       Button (part of #x86 GUI Framework)
 *
 * @date        10/01/2026
 * @version     1.0.0-beta
 */

#include <gui/button.h>

Button::Button(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char* label)
    : Widget(parent, x, y, w, h), isPressed(false) {
    this->font = FontManager::activeInstance->getNewFont();

    this->label = new char[strlen(label) + 1];
    if (!this->label) {
        HALT("CRITICAL: Failed to allocate button label!\n");
    }
    strcpy(this->label, label);

    if (this->cache) delete[] this->cache;
    if (w > 0 && h > 0) {
        this->cache = new uint32_t[this->w * this->h]();
        if (!this->cache) {
            HALT("CRITICAL: Failed to allocate button cache!\n");
        }
    }
}

Button::~Button() {
    if (label) delete[] label;
}

void Button::update() {
    MarkDirty();
}

void Button::SetLabel(const char* newLabel) {
    if (this->label) delete[] this->label;
    this->label = new char[strlen(newLabel) + 1];
    if (!this->label) {
        HALT("CRITICAL: Failed to allocate button label!\n");
    }
    strcpy(this->label, newLabel);
    MarkDirty();
}

void Button::SetWidth(int32_t reqW) {
    int32_t minW = this->font->getStringLength(label) + 4;
    this->w = (reqW < minW) ? minW : reqW;

    if (cache) delete[] cache;
    if (w > 0 && h > 0) {
        cache = new uint32_t[this->w * this->h]();
        if (!cache) {
            HALT("CRITICAL: Failed to allocate button cache!\n");
        }
    }
    MarkDirty();
}

void Button::SetHeight(int32_t reqH) {
    int32_t minH = this->font->getLineHeight() + 4;
    this->h = (reqH < minH) ? minH : reqH;

    if (cache) delete[] cache;
    if (w > 0 && h > 0) {
        cache = new uint32_t[this->w * this->h]();
        if (!cache) {
            HALT("CRITICAL: Failed to allocate button cache!\n");
        }
    }
    MarkDirty();
}

void Button::RedrawToCache() {
    uint32_t bgColor = isPressed ? BUTTON_BACKGROUND_COLOR_PRESSED : BUTTON_BACKGROUND_COLOR_NORMAL;
    uint32_t borderColor = isPressed ? BUTTON_BORDER_COLOR_PRESSED : BUTTON_BORDER_COLOR_NORMAL;
    uint32_t textColor = isPressed ? BUTTON_TEXT_COLOR_PRESSED : BUTTON_TEXT_COLOR_NORMAL;

    NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, w, h, 3, bgColor);
    NINA::activeInstance->DrawRoundedRectangle(cache, w, h, 0, 0, w, h, 3, borderColor);

    int textX = (w - this->font->getStringLength(label)) / 2;
    int textY = (h - this->font->getLineHeight()) / 2;

    NINA::activeInstance->DrawString(cache, w, h, textX, textY, label, font, textColor);

    isDirty = false;
}

void Button::OnMouseDown(int32_t, int32_t, uint8_t) {
    if (!isVisible) return;
    isPressed = true;
    MarkDirty();
}

void Button::OnMouseUp(int32_t x, int32_t y, uint8_t) {
    if (!isVisible) return;

    if (isPressed) {
        isPressed = false;
        MarkDirty();

        Event* new_event = new Event{this->ID, ON_CLICK};
        if (!new_event) {
            HALT("CRITICAL: Failed to allocate button click event!\n");
        }
        Desktop::activeInstance->getHandler(this->PID)->eventQueue.Add(new_event);
    }
}

void Button::OnMouseMove(int32_t x, int32_t y, int32_t newx, int32_t newy) {
    // Coordinates newx and newy are parent-relative (window coordinates)
    // Use ContainsCoordinate for accurate hit testing
    bool inside = this->ContainsCoordinate(newx, newy);

    // If the mouse dragged OUTSIDE the button, release the press visual
    if (isPressed && !inside) {
        isPressed = false;
        MarkDirty();
    }
}
