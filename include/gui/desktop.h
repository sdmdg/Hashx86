#ifndef DESKTOP_H
#define DESKTOP_H

#include <gui/widget.h>
#include <core/drivers/mouse.h>
#include <core/drivers/keyboard.h>
#include <gui/images/wallpaper.h>

class Desktop : public CompositeWidget, public MouseEventHandler,  public KeyboardEventHandler
{
protected:
    uint32_t MouseX;
    uint32_t MouseY;
    uint32_t current_id = 1000;
    
public:
    static Desktop* activeInstance;
    Desktop(int32_t w, int32_t h);
    ~Desktop();
    
    void Draw(GraphicsContext* gc);
    uint32_t getNewID();
    void RemoveAppByPID(uint32_t PID);
    
    void OnMouseDown(uint8_t button);
    void OnMouseUp(uint8_t button);
    void OnMouseMove(int32_t dx, int32_t dy);

    void OnKeyDown(const char* key);
    void OnSpecialKeyDown(uint8_t key);
    void OnKeyUp(const char* key);
    void OnSpecialKeyUp(uint8_t key);
};

struct DesktopArgs {
    VESA_BIOS_Extensions* vbe;
    Desktop* desktop;
};


#endif // DESKTOP_H