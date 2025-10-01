/**
 * @file        button.cpp
 * @brief       Button (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/button.h>

Button::Button(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char* label)
    : Widget(parent, x, y, w, h), isPressed(false)
    {
    this->font = FontManager::activeInstance->getNewFont();
    this->label = new char[strlen(label) + 1];
    strcpy(this->label, label);

    delete[] cache;
    cache = new uint32_t[this->w * this->h];
}

Button::~Button() {
    delete[] cache;
    delete[] label;
}

void Button::update()
{
    // Clear the cache
    memset(cache, 0, sizeof(uint32_t) * w * h);
    
    isDirty = true;
}

void Button::SetLabel(const char* label)
{
    delete[] this->label;
    this->label = new char[strlen(label) + 1];
    strcpy(this->label, label);
}

void Button::SetWidth(int32_t w)
{
    if (w < this->font->getStringLength(label)){
        this->w = this->font->getStringLength(label) + 4;
    } else {
        this->w = w;
    }
    
}

void Button::SetHeight(int32_t h)
{
    if (h < this->font->getLineHeight() ){
        this->h = this->font->getLineHeight() + 4 ;
    } else {
        this->h = h;
    }
}

void Button::RedrawToCache()
{
    //DEBUG_LOG("Button %d: Updating", this->ID);
    NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, w, h, 3, isPressed ? BUTTON_BACKGROUND_COLOR_PRESSED : BUTTON_BACKGROUND_COLOR_NORMAL);
    NINA::activeInstance->DrawRoundedRectangle(cache, w, h, 0, 0, w, h, 3, isPressed ? BUTTON_BORDER_COLOR_PRESSED : BUTTON_BORDER_COLOR_NORMAL);
    NINA::activeInstance->DrawString(cache, w, h, (w - this->font->getStringLength(label))/2 , 4, label, font, isPressed ? BUTTON_TEXT_COLOR_PRESSED : BUTTON_TEXT_COLOR_NORMAL);
    
    isDirty = false;
}

void Button::Draw(GraphicsContext* gc)
{
    if (isVisible){
        Widget::Draw(gc);
    }
}

void Button::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
    if (isVisible)
    isPressed = true;
    update();
}

void Button::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
    if (isPressed & isVisible) {
        isPressed = false;
        update();
        Event* new_event = new Event{this->ID, ON_CLICK};
        ProcessManager::activeInstance->getProcessByPID(this->PID)->eventQueue.Add(new_event);
        // DEBUG_LOG("Event added with %d, %d, addres 0x%x", new_event->widgetID, new_event->eventType, new_event);
    }
}

void Button::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy)
{
    if(!(this->ContainsCoordinate(newx, newy)) & isPressed & isVisible){
        isPressed = false;
        update();
    }
}