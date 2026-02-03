/**
 * @file        widget.cpp
 * @brief       Base Widget System (part of #x86 GUI Framework)
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <gui/widget.h>

// ============================================================================
// WIDGET BASE CLASS
// ============================================================================

Widget::Widget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h) {
    this->parent = parent;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;

    // Allocate the back buffer for this widget
    // Note: Calloc (via new[]()) ensures buffer is zeroed, which is safer.
    this->cache = new uint32_t[w * h]();
}

Widget::~Widget() {
    if (cache) delete[] cache;
}

void Widget::MarkDirty() {
    this->isDirty = true;

    // Propagate the dirty flag up the tree so the Desktop knows to redraw
    if (this->parent != nullptr) {
        this->parent->MarkDirty();
    }
}

void Widget::RedrawToCache() {
    // Default: Clear cache to transparent/black
    // Override this in child classes (e.g., Button, Label)
    if (cache) memset(cache, 0, sizeof(uint32_t) * w * h);
}

void Widget::Draw(GraphicsDriver* gc) {
    // If we are dirty, we update our internal cache.
    // The actual "blitting" to the screen usually happens in the Parent's draw loop
    // or via the Desktop.
    if (isDirty) {
        if (isVisible) {
            RedrawToCache();
        } else {
            // If hidden, clear the cache to avoid ghosting
            if (cache) memset(cache, 0, sizeof(uint32_t) * w * h);
        }
        isDirty = false;
    }
}

// -- Coordinates --

void Widget::ModelToScreen(int32_t& x_out, int32_t& y_out) {
    // Recursively calculate absolute screen coordinates
    if (parent) parent->ModelToScreen(x_out, y_out);
    x_out += this->x;
    y_out += this->y;
}

bool Widget::ContainsCoordinate(int32_t targetX, int32_t targetY) {
    // Checks if a *local* coordinate is inside this widget
    // NOTE: Input x,y should be relative to the parent
    return (targetX >= this->x) && (targetX < this->x + this->w) && (targetY >= this->y) &&
           (targetY < this->y + this->h);
}

// -- Focus --

void Widget::GetFocus(Widget* widget) {
    if (parent) parent->GetFocus(widget);
}

void Widget::SetFocus(bool result) {
    this->isFocused = result;
    // Redraw to show focus ring/highlight
    this->MarkDirty();
}

void Widget::SetFocussable(bool focussable) {
    this->isFocussable = focussable;
}

// -- Child Management --

bool Widget::AddChild(Widget* child) {
    // No arbitrary limit (100) needed if LinkedList is dynamic
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

// -- Identification --

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

// -- Default Input Handlers --

void Widget::OnMouseDown(int32_t, int32_t, uint8_t) {
    if (isFocussable) GetFocus(this);
}
void Widget::OnMouseUp(int32_t, int32_t, uint8_t) {}
void Widget::OnMouseMove(int32_t, int32_t, int32_t, int32_t) {}
void Widget::OnKeyDown(const char*) {}
void Widget::OnSpecialKeyDown(uint8_t) {}
void Widget::OnKeyUp(const char*) {}
void Widget::OnSpecialKeyUp(uint8_t) {}

// ============================================================================
// COMPOSITE WIDGET
// ============================================================================

CompositeWidget::CompositeWidget(CompositeWidget* parent, int32_t x, int32_t y, int32_t w,
                                 int32_t h)
    : Widget(parent, x, y, w, h), focusedChild(nullptr) {}

CompositeWidget::~CompositeWidget() {}

void CompositeWidget::GetFocus(Widget* widget) {
    // Deselect previous
    if (focusedChild && focusedChild != widget) {
        focusedChild->SetFocus(false);
    }

    // Select new
    focusedChild = widget;
    if (widget) {
        widget->SetFocus(true);
        // Move to front (Render order usually follows list order)
        // Optimization: Removing and re-adding can be expensive on large lists.
        // For now, it's fine as it ensures Z-indexing.
        childrenList.Remove([&](Widget* c) { return c == widget; });
        childrenList.PushBack(widget);
    }

    if (parent) parent->GetFocus(this);
}

void CompositeWidget::Draw(GraphicsDriver* gc) {
    // Draw self first
    Widget::Draw(gc);

    // Draw children from back to front
    // Note: If you want Z-order, iterate normally. If ReverseForEach meant "Top first",
    // standard drawing should be bottom-up (first in list = back).
    // Assuming AddChild appends to end (Top), we iterate normally.
    childrenList.ForEach([&](Widget* child) { child->Draw(gc); });
}

void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    // Transform Global coordinates to Local
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    Widget* clicked = nullptr;

    // Hit testing: Iterate backwards (Front-to-Back) to catch the top-most widget first
    // Note: If your list is ordered Back-to-Front, you must iterate Reverse to click top items.
    // Assuming Add() puts items at end (Top):
    childrenList.ReverseForEach([&](Widget* child) {
        if (!clicked && child->ContainsCoordinate(localX, localY)) {
            child->OnMouseDown(localX, localY, button);
            clicked = child;
        }
    });

    if (clicked) {
        GetFocus(clicked);
    } else {
        // Clicked on background of composite widget
        /* if (focusedChild) focusedChild->SetFocus(false);
           focusedChild = nullptr; */
    }
}

void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    // Transform
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    childrenList.ForEach([&](Widget* child) {
        // We pass the Up event to the child even if mouse moved out,
        // to handle "drag release" logic correctly.
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
        // Notify child if mouse is inside, OR was inside (exiting)
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
