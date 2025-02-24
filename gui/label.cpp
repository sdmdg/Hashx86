/**
 * @file        label.cpp
 * @brief       Label (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/label.h>

Label::Label(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text)
    : Widget(parent, x, y, w, h), text(text) {
        this->y += 10;
        this->w = this->font->getStringLength(text) + 10;
        this->h = 6 + font->chrHeight;
    }

Label::~Label() {}

void Label::SetText(const char* text) {
    this->text = text;
}

void Label::Draw(GraphicsContext* gc) {
    Widget::Draw(gc);
    int X = 0, Y = 0;
    ModelToScreen(X, Y);
    //gc->DrawString(X + 2, Y + 2, text, 0);
}
