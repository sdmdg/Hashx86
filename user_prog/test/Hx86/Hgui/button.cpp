/**
 * @file        button.cpp
 * @brief       Button (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <Hx86/Hgui/button.h>

Button::Button(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char* label)
    : Widget(parent, x, y, w, h), label(label)
{
    WidgetData data = {parent->ID, x, y, w, h, label};
    this->ID = HguiAPI(BUTTON, CREATE, (void*)&data);
}

Button::~Button()
{
}

bool Button::setText(const char* label)
{
    this->label = label;
/*     WidgetData data = {0, 0, 0, 0, 0, label};
    return HguiAPI(BUTTON, SET_TEXT, (void*)&data); */
}

bool Button::setWidth(int32_t w)
{
}

bool Button::setHeight(int32_t h)
{
}

void Button::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
}

void Button::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
}

void Button::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy)
{
}