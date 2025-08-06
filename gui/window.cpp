/**
 * @file        window.cpp
 * @brief       Window (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/window.h>

Window::Window(CompositeWidget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h)
: CompositeWidget(parent, x,y,w,h)
{
    Dragging = false;
    this->windowTitle = "Untitled";
    this->font = new SegoeUI();

    closeButton = new ACRButton(this, w - 22, 4, "x");
    closeButton->OnClick(this, [](void* instance) {
        static_cast<Window*>(instance)->OnClose();
    });
    
    this->AddChild(closeButton);
}

Window::~Window()
{
    delete font;
    delete closeButton;
}

void Window::update()
{
    // Clear the cache
    memset(cache, 0, sizeof(uint32_t) * w * h);

    isDirty = true;
}


void Window::OnClose()
{
    Event* new_event = new Event{this->ID, ON_WINDOW_CLOSE};
    ProcessManager::activeInstance->getProcessByPID(this->PID)->eventQueue.Add(new_event);
}

void Window::Draw(GraphicsContext* gc)
{
    int X = 0;
    int Y = 0;
    ModelToScreen(X,Y);

    // Draw window background
    ///gc->DrawRoundedRectangleShadow(X+10, Y+10, w-20, h-20, 36, 20, ((this == this->parent->children[0])? WINDOW_SHADOW_COLOR_FOCUSED : WINDOW_SHADOW_COLOR_NORMAL));
    for (auto& child : childrenList) {
        if (child->isDirty) {
            this->isDirty = true;
        }  
    }
    
    if(isDirty){
        if (isVisible)
        {
            // Only redraw if dirty
            RedrawToCache(); 
            isDirty = false;
        } else {
            // Clear the cache
            memset(cache, 0, sizeof(uint32_t) * w * h);
        }
    } 
    
    gc->DrawBitmap(X, Y, (const uint32_t*)cache, w, h);
}

void Window::RedrawToCache()
{
    // Draw self to cache
    //DEBUG_LOG("Window %d: Updating", this->ID);
    NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, w, h, 6, WINDOW_BACKGROUND_COLOR);
    
    // Draw title bar
    NINA::activeInstance->DrawBitmap(cache, w, h, 4, 2, (const uint32_t*)icon_main_20x20, 20, 20);
    NINA::activeInstance->DrawString(cache, w, h, 28, 8, windowTitle, font, WINDOW_TITLE_COLOR);

    // Draw children into this widget's cache
    for (auto& child : childrenList) {
        if (child->isDirty && child->isVisible) {
            child->RedrawToCache();
        }

        NINA::activeInstance->DrawBitmapToBuffer(
            cache, w, h,
            child->x, child->y,
            child->cache,
            child->w, child->h
        );
    }

    isDirty = false;
}

void Window::setVisible(bool val){
    this->isVisible = val;
    closeButton->isVisible = val;
    
}

void Window::setWindowTitle(const char* title){
    delete[] this->windowTitle;
    this->windowTitle = new char[strlen(title) + 1];
    strcpy(this->windowTitle, title);
    update();
}

void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
    if (this->x < x & this->y < y & this->x + w - 26 > x & this->y + 25 > y){
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