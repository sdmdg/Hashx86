/**
 * @file        widget.cpp
 * @brief       Widget (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/widget.h>

Widget::Widget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    this->parent = parent;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->colorIndex = 0; // default
    this->Focussable = true;
    this->isFocussed = false;
    cache = new uint32_t[w * h]();
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
    // Placeholder for widget-specific rendering
}

void Widget::GetFocus(Widget* widget)
{
    if (parent)
        parent->GetFocus(widget);
}

void Widget::setFocus(bool result)
{
    this->isFocussed = result;
    this->MarkDirty();
}

void Widget::SetFocussable(bool focussable)
{
    this->Focussable = focussable;
}

void Widget::SetPID(uint32_t PID)
{
    this->PID = PID;
    childrenList.ForEach([&](Widget* c) {
        c->SetPID(PID);
    });
}

void Widget::SetID(uint32_t ID)
{
    this->ID = ID;
}

Widget* Widget::FindWidgetByID(uint32_t searchID)
{
    if (this->ID == searchID) return this;

    Widget* result = nullptr;
    childrenList.ForEach([&](Widget* c) {
        if (!result) result = c->FindWidgetByID(searchID);
    });
    return result;
}

Widget* Widget::FindWidgetByPID(uint32_t PID)
{
    if (this->PID == PID) return this;

    Widget* result = nullptr;
    childrenList.ForEach([&](Widget* c) {
        if (!result) result = c->FindWidgetByPID(PID);
    });
    return result;
}

bool Widget::AddChild(Widget* child)
{
    if (childrenList.GetSize() >= 100) return false;
    childrenList.Add(child);
    return true;
}

bool Widget::RemoveChild(Widget* child)
{
    return childrenList.Remove([&](Widget* c) { return c == child; });
}

void Widget::ModelToScreen(int32_t& x, int32_t& y)
{
    if (parent) parent->ModelToScreen(x, y);
    x += this->x;
    y += this->y;
}

bool Widget::ContainsCoordinate(int32_t x, int32_t y)
{
    return this->x <= x && x < this->x + this->w &&
           this->y <= y && y < this->y + this->h;
}

void Widget::Draw(GraphicsContext* gc)
{
    if (isDirty) {
        if (isVisible) {
            RedrawToCache();
        } else {
            memset(cache, 0, sizeof(uint32_t) * w * h);
        }
        isDirty = false;
    }
}

void Widget::OnMouseDown(int32_t, int32_t, uint8_t)
{
    if (Focussable) GetFocus(this);
}

void Widget::OnMouseUp(int32_t, int32_t, uint8_t) {}
void Widget::OnMouseMove(int32_t, int32_t, int32_t, int32_t) {}

void Widget::OnKeyDown(const char*) {}
void Widget::OnSpecialKeyDown(uint8_t) {}
void Widget::OnKeyUp(const char*) {}
void Widget::OnSpecialKeyUp(uint8_t) {}


// ---------------- CompositeWidget ----------------

CompositeWidget::CompositeWidget(CompositeWidget* parent, int32_t x, int32_t y, int32_t w, int32_t h)
    : Widget(parent, x, y, w, h), focussedChild(nullptr) {}

CompositeWidget::~CompositeWidget() {}

void CompositeWidget::GetFocus(Widget* widget)
{
    if (focussedChild && focussedChild != widget) {
        focussedChild->setFocus(false);
    }
    focussedChild = widget;
    if (widget) widget->setFocus(true);

    if (parent) parent->GetFocus(this);
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
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    Widget* clicked = nullptr;
    childrenList.ForEach([&](Widget* child) {
        if (!clicked && child->ContainsCoordinate(localX, localY)) {
            child->OnMouseDown(localX, localY, button);
            clicked = child;
        }
    });

    if (clicked) {
        childrenList.Remove([&](Widget* c) { return c == clicked; });
        childrenList.Add(clicked);
        GetFocus(clicked);
    } else {
        if (focussedChild) focussedChild->setFocus(false);
        focussedChild = nullptr;
    }
}

void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
    childrenList.ForEach([&](Widget* child) {
        if (child->ContainsCoordinate(x - this->x, y - this->y)) {
            child->OnMouseUp(x - this->x, y - this->y, button);
        }
    });
}

void CompositeWidget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy)
{
    childrenList.ForEach([&](Widget* child) {
        bool inOld = child->ContainsCoordinate(oldx - this->x, oldy - this->y);
        bool inNew = child->ContainsCoordinate(newx - this->x, newy - this->y);
        if (inOld || inNew) {
            child->OnMouseMove(oldx - this->x, oldy - this->y,
                               newx - this->x, newy - this->y);
        }
    });
}

void CompositeWidget::OnKeyDown(const char* key)
{
    if (focussedChild) focussedChild->OnKeyDown(key);
    else Widget::OnKeyDown(key);
}

void CompositeWidget::OnKeyUp(const char* key)
{
    if (focussedChild) focussedChild->OnKeyUp(key);
    else Widget::OnKeyUp(key);
}

void CompositeWidget::OnSpecialKeyDown(uint8_t key)
{
    if (focussedChild) focussedChild->OnSpecialKeyDown(key);
    else Widget::OnSpecialKeyDown(key);
}

void CompositeWidget::OnSpecialKeyUp(uint8_t key)
{
    if (focussedChild) focussedChild->OnSpecialKeyUp(key);
    else Widget::OnSpecialKeyUp(key);
}
