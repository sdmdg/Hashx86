/**
 * @file        desktop.cpp
 * @brief       Desktop (part of #x86 GUI Framework)
 *
 * @date        10/01/2026
 * @version     1.0.0-beta
 */

#include <gui/desktop.h>

Desktop* Desktop::activeInstance = nullptr;

Desktop::Desktop(int32_t w, int32_t h)
    : CompositeWidget(0, 0, 0, w, h), MouseEventHandler(), KeyboardEventHandler() {
    MouseX = w / 2;
    MouseY = h / 2;
    activeInstance = this;

    DEBUG_LOG("DESKTOP Initialized with ID 0x%x", this->ID);

    // Initialize Wallpaper
    char* wallpaperName = (char*)"BITMAPS/DESKTOP.BMP";
    Bitmap* wallpaperImg = new Bitmap(wallpaperName);

    if (wallpaperImg->IsValid()) {
        this->Wallpaper = wallpaperImg;
    } else {
        // Fallback: Solid Color (Blue-ish)
        this->Wallpaper = new Bitmap(w, h, 0xFF0000FF);
        delete wallpaperImg;
    }

    memset(cursorBackBuffer, 0, sizeof(cursorBackBuffer));
}

Desktop::~Desktop() {
    if (Wallpaper) delete Wallpaper;
}

void Desktop::createNewHandler(uint32_t pid, ThreadControlBlock* thread) {
    EventHandler* handler = new EventHandler{};
    handler->pid = pid;
    handler->thread = thread;
    HguiEventHandlers.Add(handler);
}

void Desktop::deleteEventHandler(uint32_t pid) {
    HguiEventHandlers.Remove([&](EventHandler* e) { return e->pid == pid; });
}

EventHandler* Desktop::getHandler(uint32_t pid) {
    EventHandler* _eventHandler = nullptr;
    HguiEventHandlers.ForEach([&](EventHandler* e) {
        if (e->pid == pid) _eventHandler = e;
    });
    return _eventHandler;
}

void Desktop::Draw(GraphicsDriver* gc) {
    InterruptGuard guard;
    uint32_t screenW = gc->GetWidth();
    uint32_t screenH = gc->GetHeight();
    uint32_t* vesaBuffer = gc->GetBackBuffer();

    // -----------------------------------------------------------------
    // CASE 1: FULL SYSTEM REDRAW
    // Triggered when a window moves, opens, closes, or invalidates the desktop.
    // -----------------------------------------------------------------
    if (this->isDirty) {
        // 1. Draw Wallpaper
        gc->DrawBitmap(0, 0, (const uint32_t*)this->Wallpaper->GetBuffer(),
                       this->Wallpaper->GetWidth(), this->Wallpaper->GetHeight());

        // 2. Composite all Children (Windows) onto the screen
        // Calls CompositeWidget::Draw -> calls Window::Draw
        CompositeWidget::Draw(gc);

        // 3. Save Background under Mouse (Fresh capture)
        for (int y = 0; y < CURSOR_SIZE; y++) {
            for (int x = 0; x < CURSOR_SIZE; x++) {
                // Bounds Check!
                int destX = MouseX + x;
                int destY = MouseY + y;

                if (destX >= (int)screenW || destY >= (int)screenH) continue;

                cursorBackBuffer[y * CURSOR_SIZE + x] = vesaBuffer[destY * screenW + destX];
            }
        }
        hasBackBuffer = true;

        // 4. Draw Cursor
        gc->DrawBitmap(MouseX, MouseY, (const uint32_t*)icon_cursor_20x20, CURSOR_SIZE,
                       CURSOR_SIZE);

        // 5. Reset State
        this->isDirty = false;
        oldMouseX = MouseX;
        oldMouseY = MouseY;
        return;
    }

    // -----------------------------------------------------------------
    // CASE 2: MOUSE MOVEMENT ONLY
    // Optimization: If nothing else changed, just undraw/redraw cursor.
    // -----------------------------------------------------------------
    if (MouseX != oldMouseX || MouseY != oldMouseY) {
        // A. ERASE OLD CURSOR (Restore saved pixels)
        if (hasBackBuffer) {
            for (int y = 0; y < CURSOR_SIZE; y++) {
                for (int x = 0; x < CURSOR_SIZE; x++) {
                    int destX = oldMouseX + x;
                    int destY = oldMouseY + y;

                    if (destX >= (int)screenW || destY >= (int)screenH) continue;

                    // Direct buffer write for speed
                    vesaBuffer[destY * screenW + destX] = cursorBackBuffer[y * CURSOR_SIZE + x];
                }
            }
        }

        // B. CAPTURE BACKGROUND AT NEW POSITION
        for (int y = 0; y < CURSOR_SIZE; y++) {
            for (int x = 0; x < CURSOR_SIZE; x++) {
                int destX = MouseX + x;
                int destY = MouseY + y;

                if (destX >= (int)screenW || destY >= (int)screenH) continue;

                cursorBackBuffer[y * CURSOR_SIZE + x] = vesaBuffer[destY * screenW + destX];
            }
        }
        hasBackBuffer = true;

        // C. DRAW NEW CURSOR
        gc->DrawBitmap(MouseX, MouseY, (const uint32_t*)icon_cursor_20x20, CURSOR_SIZE,
                       CURSOR_SIZE);

        // Update History
        oldMouseX = MouseX;
        oldMouseY = MouseY;
    }
}

uint32_t Desktop::getNewID() {
    return current_id++;
}

void Desktop::RemoveAppByPID(uint32_t pid) {
    Widget* result = nullptr;
    childrenList.ForEach([&](Widget* c) {
        if (!result && c->PID == pid) result = c;
    });

    if (result) {
        this->RemoveChild(result);
        this->MarkDirty();  // Ensure screen clears the removed window
    }
}

// -- Inputs --

void Desktop::OnMouseDown(uint8_t button) {
    CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseUp(uint8_t button) {
    CompositeWidget::OnMouseUp(MouseX, MouseY, button);
}

void Desktop::OnMouseMove(int32_t dx, int32_t dy) {
    MouseX += dx;
    MouseY += dy;

    // Clamp mouse position to screen bounds
    if (MouseX < 0) MouseX = 0;
    if (MouseY < 0) MouseY = 0;
    if (MouseX >= (uint32_t)w) MouseX = w - 1;
    if (MouseY >= (uint32_t)h) MouseY = h - 1;

    // Pass delta to UI
    CompositeWidget::OnMouseMove(MouseX - dx, MouseY - dy, MouseX, MouseY);
}

void Desktop::OnKeyDown(const char* key) {
    CompositeWidget::OnKeyDown(key);
}
void Desktop::OnKeyUp(const char* key) {
    CompositeWidget::OnKeyUp(key);
}
void Desktop::OnSpecialKeyDown(uint8_t key) {
    CompositeWidget::OnSpecialKeyDown(key);
}
void Desktop::OnSpecialKeyUp(uint8_t key) {
    CompositeWidget::OnSpecialKeyUp(key);
}
