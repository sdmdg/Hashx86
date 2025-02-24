/**
 * @file        desktop.cpp
 * @brief       Desktop (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/desktop.h>


Desktop::Desktop(int32_t w, int32_t h)
    : CompositeWidget(0, 0, 0, w, h), MouseEventHandler() {
    MouseX = w / 2;
    MouseY = h / 2;
}

Desktop::~Desktop() {}

void Desktop::Draw(GraphicsContext* gc) {
    // Draw the desktop background
    gc->DrawBitmap(0, 0, (const uint32_t*)image_wallpaper_1024x768, 1024, 768);

    // Draw all windows and widgets
    CompositeWidget::Draw(gc);

    // Draw taskbar
    //gc->BlurRoundedRectangle(100, GUI_SCREEN_HEIGHT - 50, w-200, 50, 20, 6);
    gc->FillRoundedRectangle(362, GUI_SCREEN_HEIGHT - 50, 300, 50, 10, DESKTOP_TASKBAR_BACKGROUND_COLOR);

    // Draw the mouse cursor
    gc->DrawBitmap(MouseX, MouseY, (const uint32_t*)icon_cursor_20x20, 20, 20);
}

void Desktop::OnMouseDown(uint8_t button) {
    CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseUp(uint8_t button) {
    CompositeWidget::OnMouseUp(MouseX, MouseY, button);
}

void Desktop::OnMouseMove(int32_t dx, int32_t dy) {
    MouseX += dx;
    MouseY += dy;

    // Clamp mouse position
    if (MouseX < 0) MouseX = 0;
    if (MouseY < 0) MouseY = 0;
    if (MouseX >= w) MouseX = w - 1;
    if (MouseY >= h) MouseY = h - 1;

    CompositeWidget::OnMouseMove(MouseX - dx, MouseY - dy, MouseX, MouseY);
}
