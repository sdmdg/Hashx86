#ifndef DESKTOP_H
#define DESKTOP_H

#include <gui/widget.h>
#include <core/drivers/mouse.h>
#include <gui/images/wallpaper.h>


class Desktop : public CompositeWidget, public MouseEventHandler
{
protected:
    uint32_t MouseX;
    uint32_t MouseY;
    
public:
    Desktop(int32_t w, int32_t h);
    ~Desktop();
    
    void Draw(GraphicsContext* gc);
    
    void OnMouseDown(uint8_t button);
    void OnMouseUp(uint8_t button);
    void OnMouseMove(int32_t dx, int32_t dy) ;
};


#endif // DESKTOP_H