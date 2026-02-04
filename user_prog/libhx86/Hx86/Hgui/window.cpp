/**
 * @file        window.cpp
 * @brief       Window (part of #x86 GUI Framework)
 *
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <Hx86/Hgui/window.h>

Window::Window(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h)
    : CompositeWidget(parent, x, y, w, h) {
    WidgetData data = {parent->ID, x, y, w, h};
    this->ID = HguiAPI(WINDOW, CREATE, (void*)&data);
}

Window::~Window() {}

void Window::show() {
    this->parent->AddChild(this);
}

void Window::OnCloseButton() {}

void Window::setWindowTitle(const char* title) {
    WidgetData data = {ID, 0, 0, 0, 0, title};
    HguiAPI(WINDOW, SET_TEXT, (void*)&data);
}

void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button) {}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button) {}

void Window::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {}
