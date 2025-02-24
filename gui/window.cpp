/**
 * @file        window.cpp
 * @brief       Window (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/window.h>

Window::Window(Widget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h)
: CompositeWidget(parent, x,y,w,h)
{
    Dragging = false;
    this->windowTitle = "Untitled";
}

Window::~Window()
{
}

void Window::Draw(GraphicsContext* gc)
{
    int X = 0;
    int Y = 0;
    ModelToScreen(X,Y);

    // Draw window background
    gc->FillRoundedRectangle(X, Y, w, h, 6, WINDOW_BACKGROUND_COLOR);

    // Draw title bar
    //gc->FillRoundedRectangle(X, Y, w, 25, 6, 0xFFFF2020);
    gc->DrawBitmap(X + 4, Y + 2, (const uint32_t*)icon_main_20x20, 20, 20);
    gc->DrawString(X + 28, Y + 8, this->windowTitle, this->font, WINDOW_TITLE_COLOR);

    // Draw children (below title bar)
    for (int i = numChildren - 1; i >= 0; --i) {
        children[i]->Draw(gc);
    }
}

void Window::setWindowTitle(const char* title){
    this->windowTitle = title;
}

void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
    if (this->x < x & this->y < y & this->x + w > x & this->y + 25 > y){
        Dragging = button == 1;
    }
    CompositeWidget::OnMouseDown(x,y,button);
}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
    Dragging = false;
    CompositeWidget::OnMouseUp(x,y,button);
}

void Window::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy)
{
    if(Dragging)
    {
        int32_t tmp_x = x + newx-oldx;
        int32_t tmp_y = y + newy-oldy;
 
        this->x = tmp_x;
        this->y = tmp_y;
    };
    CompositeWidget::OnMouseMove(oldx,oldy,newx, newy);

}