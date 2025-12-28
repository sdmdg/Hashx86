/**
 * @file        desktop.cpp
 * @brief       Desktop (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/desktop.h>

Desktop* Desktop::activeInstance = nullptr;

Desktop::Desktop(int32_t w, int32_t h)
    : CompositeWidget(0, 0, 0, w, h), MouseEventHandler(), KeyboardEventHandler()
{
    MouseX = w / 2;
    MouseY = h / 2;
    activeInstance = this;
    DEBUG_LOG("DESKTOP id 0x%x", this->ID);

    char * wallpaperName = (char *)"BITMAPS/DESKTOP.BMP";
    Bitmap* wallpaperImg = new Bitmap(wallpaperName);
    if (wallpaperImg->IsValid()) {
        this->Wallpaper = wallpaperImg;
    } else {
        this->Wallpaper = new Bitmap(w, h, 0xFF0000FF);
        delete wallpaperImg;
    }
}

Desktop::~Desktop()
{
}

void Desktop::Draw(GraphicsDriver* gc)
{
    // Draw the desktop background
    gc->DrawBitmap(0, 0, (const uint32_t*)this->Wallpaper->GetBuffer(), this->Wallpaper->GetWidth(), this->Wallpaper->GetHeight());

    // Draw all windows and widgets
    CompositeWidget::Draw(gc);

    // Draw taskbar
    //gc->FillRoundedRectangle(362, GUI_SCREEN_HEIGHT - 50, 300, 50, 10, DESKTOP_TASKBAR_BACKGROUND_COLOR);

    // Draw the mouse cursor
    gc->DrawBitmap(MouseX, MouseY, (const uint32_t*)icon_cursor_20x20, 20, 20);
}

uint32_t Desktop::getNewID()
{
    return current_id++;
}

void Desktop::RemoveAppByPID(uint32_t PID)
{
    Widget* result = nullptr;
    childrenList.ForEach([&](Widget* c) {
        if (result) return;

        if (c->PID == PID)
            result = c;
    });
    if (result)
        this->RemoveChild(result);
}

void Desktop::OnMouseDown(uint8_t button)
{
    CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseUp(uint8_t button)
{
    CompositeWidget::OnMouseUp(MouseX, MouseY, button);
}

void Desktop::OnMouseMove(int32_t dx, int32_t dy)
{
    MouseX += dx;
    MouseY += dy;

    // Clamp mouse position
    if (MouseX < 1) MouseX = 1;
    if (MouseY < 1) MouseY = 1;
    if (MouseX >= w) MouseX = w - 1;
    if (MouseY >= h) MouseY = h - 1;

    CompositeWidget::OnMouseMove(MouseX - dx, MouseY - dy, MouseX, MouseY);
}


void Desktop::OnKeyDown(const char* key)
{
    CompositeWidget::OnKeyDown(key);
};
void Desktop::OnKeyUp(const char* key)
{
    CompositeWidget::OnKeyUp(key);
};


void Desktop::OnSpecialKeyDown(uint8_t key)
{
    
    DEBUG_LOG("Key Pressed 0x%x", key);
};
void Desktop::OnSpecialKeyUp(uint8_t key)
{
    DEBUG_LOG("Key Pressed 0x%x", key);
};