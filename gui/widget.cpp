/**
 * @file        widget.cpp
 * @brief       Widget (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/widget.h>

Widget::Widget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h)
: KeyboardEventHandler()
{
    this->parent = parent;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->colorIndex = colorIndex;
    this->Focussable = true;
    cache = new uint32_t[w * h];
}

Widget::~Widget()
{
    delete[] cache;
}

void Widget::MarkDirty()
{
    isDirty = true;
    if (parent) parent->MarkDirty(); // propagate upward
}

void Widget::RedrawToCache()
{
}

void Widget::GetFocus(Widget* widget)
{
    if(parent != 0)
        parent->GetFocus(widget);
}

void Widget::SetFocussable(bool focussable)
{
    this->Focussable = focussable;
}

void Widget::SetPID(uint32_t PID)
{
    this->PID = PID;
    childrenList.ForEach([&](Widget* c) {
        c->PID = PID;
    });
}

void Widget::SetID(uint32_t ID)
{
    this->ID = ID;
}

Widget* Widget::FindWidgetByID(uint32_t searchID)
{
    if (this->ID == searchID)
        return this;

    Widget* result = nullptr;

    childrenList.ForEach([&](Widget* c) {
        if (result) return; // Already found

        if (c->ID == searchID)
            result = c;
        else {
            Widget* nested = c->FindWidgetByID(searchID);
            if (nested)
                result = nested;
        }
    });

    return result;
}

Widget* Widget::FindWidgetByPID(uint32_t PID)
{
    if (this->PID == PID)
        return this;

    Widget* result = nullptr;

    childrenList.ForEach([&](Widget* c) {
        if (result) return; // Already found

        if (c->PID == PID)
            result = c;
        else {
            Widget* nested = c->FindWidgetByID(PID);
            if (nested)
                result = nested;
        }
    });

    return result;
}

bool Widget::AddChild(Widget* child)
{
    if (childrenList.GetSize() >= 100)
        return false;

    // Add to front
    childrenList.Add(child);
    return true;
}



bool Widget::RemoveChild(Widget* child)
{
    return childrenList.Remove([&](Widget* c) {
        return c == child;
    });
}

void Widget::ModelToScreen(int32_t &x, int32_t& y)
{
    if(parent != 0)
        parent->ModelToScreen(x,y);
    x += this->x;
    y += this->y;
}
            
void Widget::Draw(GraphicsContext* gc)
{
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
}


void Widget::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
    if(Focussable)
        GetFocus(this);
}

bool Widget::ContainsCoordinate(int32_t x, int32_t y)
{
    return this->x <= x && x < this->x + this->w
        && this->y <= y && y < this->y + this->h;
}

void Widget::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
}

void Widget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy)
{
}





CompositeWidget::CompositeWidget(CompositeWidget* parent,
                   int32_t x, int32_t y, int32_t w, int32_t h)
: Widget(parent, x,y,w,h)
{
    focussedChild = 0;
}

CompositeWidget::~CompositeWidget()
{
}
            
void CompositeWidget::GetFocus(Widget* widget)
{
    this->focussedChild = widget;
    if (parent != nullptr)
        parent->GetFocus(this);
}

void CompositeWidget::Draw(GraphicsContext* gc)
{
    Widget::Draw(gc);
    childrenList.ReverseForEach([&](Widget* child) {
        child->Draw(gc);
    });
    
}

void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
    const int32_t localX = x - this->x;
    const int32_t localY = y - this->y;

    // One loop: Find + trigger + move to front
    Widget* clicked = nullptr;

    childrenList.ForEach([&](Widget* child) {
        if (!clicked && child->ContainsCoordinate(localX, localY)) {
            child->OnMouseDown(localX, localY, button);
            clicked = child;
        }
    });

    if (clicked) {
        // Modify only *after* iteration
        childrenList.Remove([&](Widget* c) { return c == clicked; });
        childrenList.Add(clicked); // Bring to front
    }
}


void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
    for (auto it = childrenList.begin(); it != childrenList.end(); ++it) {
        Widget* child = *it;
        if (child->ContainsCoordinate(x - this->x, y - this->y)) {
            child->OnMouseUp(x - this->x, y - this->y, button);
            break;
        }
    }
}

void CompositeWidget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy)
{
    bool handledOld = false;
    bool handledNew = false;

    for (auto it = childrenList.begin(); it != childrenList.end(); ++it) {
        Widget* child = *it;
        bool inOld = child->ContainsCoordinate(oldx - this->x, oldy - this->y);
        bool inNew = child->ContainsCoordinate(newx - this->x, newy - this->y);

        if (inOld || inNew) {
            child->OnMouseMove(oldx - this->x, oldy - this->y, newx - this->x, newy - this->y);
        }

        if (inOld) handledOld = true;
        if (inNew) handledNew = true;

        if (handledOld && handledNew) break;
    }
}
