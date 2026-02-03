/**
 * @file        window.cpp
 * @brief       Window Component (part of #x86 GUI Framework)
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <gui/window.h>

Window::Window(CompositeWidget* parent, int32_t x, int32_t y, int32_t w, int32_t h)
    : CompositeWidget(parent, x, y, w, h) {
    this->isDragging = false;
    this->windowTitle = new char[10];
    strcpy(this->windowTitle, "Untitled");
    this->font = FontManager::activeInstance->getNewFont();

    closeButton = new ACRButton(this, w - 22, 4, "x");
    closeButton->OnClick(this, [](void* instance) { static_cast<Window*>(instance)->OnClose(); });

    this->AddChild(closeButton);
}

Window::~Window() {
    delete[] windowTitle;
    delete closeButton;
    if (font) delete font;
}

void Window::OnClose() {
    if (this->parent) this->parent->MarkDirty();

    Event* new_event = new Event{this->ID, ON_WINDOW_CLOSE};
    Desktop::activeInstance->getHandler(this->PID)->eventQueue.Add(new_event);
}

void Window::setWindowTitle(const char* title) {
    if (windowTitle) delete[] windowTitle;
    windowTitle = new char[strlen(title) + 1];
    strcpy(windowTitle, title);
    MarkDirty();
}

void Window::setVisible(bool val) {
    this->isVisible = val;
    closeButton->isVisible = val;
    MarkDirty();
}

void Window::Draw(GraphicsDriver* gc) {
    int X = 0, Y = 0;
    ModelToScreen(X, Y);

    for (auto& child : childrenList) {
        if (child->isDirty) {
            this->isDirty = true;
        }
    }

    if (isDirty) {
        if (isVisible) {
            RedrawToCache();
        } else {
            memset(cache, 0, sizeof(uint32_t) * w * h);
        }
        isDirty = false;
    }

    if (isVisible) {
        gc->DrawBitmap(X, Y, (const uint32_t*)cache, w, h);
    }
}

void Window::RedrawToCache() {
    NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, w, h, 6, WINDOW_BACKGROUND_COLOR);
    NINA::activeInstance->DrawBitmap(cache, w, h, 4, 2, (const uint32_t*)icon_main_20x20, 20, 20);
    NINA::activeInstance->DrawString(cache, w, h, 28, 3, windowTitle, font, WINDOW_TITLE_COLOR);

    for (auto& child : childrenList) {
        if (!child->isVisible) continue;
        if (child->isDirty) child->RedrawToCache();

        NINA::activeInstance->DrawBitmapToBuffer(cache, w, h, child->x, child->y, child->cache,
                                                 child->w, child->h);
    }
}

void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    // Convert desktop coordinates to window-local coordinates
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    // Check Header Area (Top 25 pixels)
    if (localX >= 0 && localX <= w - 26 && localY >= 0 && localY <= 25) {
        isDragging = (button == 1);
    }

    // Pass original coordinates to base (it handles converting for children)
    CompositeWidget::OnMouseDown(x, y, button);
}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    isDragging = false;
    CompositeWidget::OnMouseUp(x, y, button);
}

void Window::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {
    if (isDragging) {
        int32_t dx = newx - oldx;
        int32_t dy = newy - oldy;

        this->x += dx;
        this->y += dy;

        if (this->parent) this->parent->MarkDirty();
    }

    CompositeWidget::OnMouseMove(oldx, oldy, newx, newy);
}
