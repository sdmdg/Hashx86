/**
 * @file        widget.cpp
 * @brief       Base Widget System (part of #x86 GUI Framework)
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <gui/widget.h>

// Widget Base Class

Widget::Widget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h) {
    this->parent = parent;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;

    // Allocate and zero-init cache buffer
    if (w > 0 && h > 0) {
        cache = new uint32_t[w * h]();
        if (!cache) {
            HALT("CRITICAL: Failed to allocate button cache!\n");
        }
    }
}

Widget::~Widget() {
    if (cache) delete[] cache;
}

void Widget::MarkDirty() {
    this->isDirty = true;

    // Propagate dirty flag upward
    if (this->parent != nullptr) {
        this->parent->MarkDirty();
    }
}

void Widget::RedrawToCache() {
    // Clear cache (override in child classes)
    if (cache) memset(cache, 0, sizeof(uint32_t) * w * h);
}

void Widget::Draw(GraphicsDriver* gc) {
    // Update cache if dirty
    if (isDirty) {
        if (isVisible) {
            RedrawToCache();
        } else {
            // Clear cache if hidden
            if (cache) memset(cache, 0, sizeof(uint32_t) * w * h);
        }
        isDirty = false;
    }
}

// Coordinates

void Widget::ModelToScreen(int32_t& x_out, int32_t& y_out) {
    // Calculate absolute screen position
    if (parent) parent->ModelToScreen(x_out, y_out);
    x_out += this->x;
    y_out += this->y;
}

bool Widget::ContainsCoordinate(int32_t targetX, int32_t targetY) {
    // Check if local coordinate is inside widget
    return (targetX >= this->x) && (targetX < this->x + this->w) && (targetY >= this->y) &&
           (targetY < this->y + this->h);
}

// Focus

void Widget::GetFocus(Widget* widget) {
    if (parent) parent->GetFocus(widget);
}

void Widget::SetFocus(bool result) {
    this->isFocused = result;
    // Redraw with focus indicator
    this->MarkDirty();
}

void Widget::SetFocussable(bool focussable) {
    this->isFocussable = focussable;
}

// Child Management

bool Widget::AddChild(Widget* child) {
    childrenList.Add(child);
    child->parent = this;
    this->MarkDirty();
    return true;
}

bool Widget::RemoveChild(Widget* child) {
    bool result = childrenList.Remove([&](Widget* c) { return c == child; });
    if (result) (this->MarkDirty());
    return result;
}

// Identification

void Widget::SetPID(uint32_t pid) {
    this->PID = pid;
    childrenList.ForEach([&](Widget* c) { c->SetPID(pid); });
}

void Widget::SetID(uint32_t id) {
    this->ID = id;
}

Widget* Widget::FindWidgetByID(uint32_t searchID) {
    if (this->ID == searchID) return this;

    Widget* result = nullptr;
    childrenList.ForEach([&](Widget* c) {
        if (!result) result = c->FindWidgetByID(searchID);
    });
    return result;
}

Widget* Widget::FindWidgetByPID(uint32_t pid) {
    if (this->PID == pid) return this;

    Widget* result = nullptr;
    childrenList.ForEach([&](Widget* c) {
        if (!result) result = c->FindWidgetByPID(pid);
    });
    return result;
}

// Default Input Handlers

void Widget::OnMouseDown(int32_t, int32_t, uint8_t) {
    if (isFocussable) GetFocus(this);
}
void Widget::OnMouseUp(int32_t, int32_t, uint8_t) {}
void Widget::OnMouseMove(int32_t, int32_t, int32_t, int32_t) {}
void Widget::OnKeyDown(const char*) {}
void Widget::OnSpecialKeyDown(uint8_t) {}
void Widget::OnKeyUp(const char*) {}
void Widget::OnSpecialKeyUp(uint8_t) {}

// Composite Widget

CompositeWidget::CompositeWidget(CompositeWidget* parent, int32_t x, int32_t y, int32_t w,
                                 int32_t h)
    : Widget(parent, x, y, w, h), focusedChild(nullptr) {}

CompositeWidget::~CompositeWidget() {}

void CompositeWidget::GetFocus(Widget* widget) {
    // Deselect previous child
    if (focusedChild && focusedChild != widget) {
        focusedChild->SetFocus(false);
    }

    // Select new child
    focusedChild = widget;
    if (widget) {
        widget->SetFocus(true);
        // Move to front for Z-order
        childrenList.Remove([&](Widget* c) { return c == widget; });
        childrenList.PushBack(widget);
    }

    if (parent) parent->GetFocus(this);
}

void CompositeWidget::Draw(GraphicsDriver* gc) {
    // Draw self
    Widget::Draw(gc);

    // Draw children back-to-front
    childrenList.ForEach([&](Widget* child) { child->Draw(gc); });
}

void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    // Transform to local coordinates
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    Widget* clicked = nullptr;

    // Hit test front-to-back
    childrenList.ReverseForEach([&](Widget* child) {
        if (!clicked && child->ContainsCoordinate(localX, localY)) {
            child->OnMouseDown(localX, localY, button);
            clicked = child;
        }
    });

    if (clicked) {
        GetFocus(clicked);
    } else {
        // Background click
        /* if (focusedChild) focusedChild->SetFocus(false);
           focusedChild = nullptr; */
    }
}

void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    // Transform to local
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    childrenList.ForEach([&](Widget* child) {
        // Pass event even if mouse moved out
        if (child->ContainsCoordinate(localX, localY)) {
            child->OnMouseUp(localX, localY, button);
        }
    });
}

void CompositeWidget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {
    int32_t localOldX = oldx - this->x;
    int32_t localOldY = oldy - this->y;
    int32_t localNewX = newx - this->x;
    int32_t localNewY = newy - this->y;

    childrenList.ForEach([&](Widget* child) {
        // Notify if mouse inside or entering/exiting
        bool inOld = child->ContainsCoordinate(localOldX, localOldY);
        bool inNew = child->ContainsCoordinate(localNewX, localNewY);

        if (inOld || inNew) {
            child->OnMouseMove(localOldX, localOldY, localNewX, localNewY);
        }
    });
}

void CompositeWidget::OnKeyDown(const char* key) {
    if (focusedChild) focusedChild->OnKeyDown(key);
}

void CompositeWidget::OnKeyUp(const char* key) {
    if (focusedChild) focusedChild->OnKeyUp(key);
}

void CompositeWidget::OnSpecialKeyDown(uint8_t key) {
    if (focusedChild) focusedChild->OnSpecialKeyDown(key);
}

void CompositeWidget::OnSpecialKeyUp(uint8_t key) {
    if (focusedChild) focusedChild->OnSpecialKeyUp(key);
}
