#ifndef DESKTOP_H
#define DESKTOP_H

#include <core/drivers/GraphicsDriver.h>
#include <core/drivers/keyboard.h>
#include <core/drivers/mouse.h>
#include <gui/bmp.h>
#include <gui/widget.h>

/**
 * @class       Desktop
 * @brief       The Root Widget. Manages the wallpaper, cursor, and global window list.
 */
class Desktop : public CompositeWidget, public MouseEventHandler, public KeyboardEventHandler {
protected:
    uint32_t MouseX;
    uint32_t MouseY;
    uint32_t current_id = 1000;
    Bitmap* Wallpaper;

    // -- Cursor Optimization Buffers --
    int32_t oldMouseX = 0;
    int32_t oldMouseY = 0;

    // Buffer to store pixels BEHIND the cursor (Max 32x32 support)
    static const int CURSOR_SIZE = 20;
    uint32_t cursorBackBuffer[CURSOR_SIZE * CURSOR_SIZE];
    bool hasBackBuffer = false;

    LinkedList<EventHandler*> HguiEventHandlers;

public:
    static Desktop* activeInstance;

    Desktop(int32_t w, int32_t h);
    ~Desktop();

    void createNewHandler(uint32_t pid, ThreadControlBlock* thread);
    void deleteEventHandler(uint32_t pid);
    EventHandler* getHandler(uint32_t pid);

    // The Master Draw function
    void Draw(GraphicsDriver* gc) override;

    uint32_t getNewID();
    void RemoveAppByPID(uint32_t PID);

    // Driver Inputs
    void OnMouseDown(uint8_t button) override;
    void OnMouseUp(uint8_t button) override;
    void OnMouseMove(int32_t dx, int32_t dy) override;

    void OnKeyDown(const char* key) override;
    void OnSpecialKeyDown(uint8_t key) override;
    void OnKeyUp(const char* key) override;
    void OnSpecialKeyUp(uint8_t key) override;

    bool MouseMoved() {
        return (MouseX != oldMouseX || MouseY != oldMouseY);
    }
};

struct DesktopArgs {
    GraphicsDriver* screen;
    Desktop* desktop;
    FAT32* boot_partition;
};

struct LoadProgArgs {
    ELFLoader* elfLoader;
    FAT32* boot_partition;
};

#endif  // DESKTOP_H
