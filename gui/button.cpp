/**
 * @file        button.cpp
 * @brief       Button (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/button.h>

Button::Button(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* label)
    : Widget(parent, x, y, w, h), label(label), isPressed(false), onClickPtr(nullptr) {
    this->y += 10;
    this->paddingX = 6;
    this->paddingY = 5;
    this->w = 2 * paddingX + this->font->getStringLength(label);
    this->h = 2 * paddingY + this->font->chrHeight;
}

Button::~Button() {}

void Button::SetLabel(const char* label) {
    this->label = label;
}

void Button::OnClick(void (*callback)()) {
    onClickPtr = callback;  // Store the callback function
}

void Button::Draw(GraphicsContext* gc) {
    int X = 0, Y = 0;
    ModelToScreen(X, Y);

    gc->FillRoundedRectangle(X, Y, w, h, 3, isPressed ? BUTTON_BACKGROUND_COLOR_PRESSED : BUTTON_BACKGROUND_COLOR_NORMAL);           // Change border color if pressed
    gc->DrawRoundedRectangle(X, Y, w, h, 3, isPressed ? BUTTON_BORDER_COLOR_PRESSED : BUTTON_BORDER_COLOR_NORMAL);                   // Change border color if pressed
    gc->DrawString(X + paddingX, Y + paddingY, label, this->font, isPressed ? BUTTON_TEXT_COLOR_PRESSED : BUTTON_TEXT_COLOR_NORMAL); // Center the label text
}

void Button::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    isPressed = true;
}

void Button::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    if (isPressed){
        isPressed = false;
        if (onClickPtr) {
            onClickPtr();
        }
    }

}

void Button::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy){
    if(!(this->ContainsCoordinate(newx, newy)) & isPressed){
        isPressed = false;
    }
}