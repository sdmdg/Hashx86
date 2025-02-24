#ifndef WINDOW_H
#define WINDOW_H

#include <gui/widget.h>

class Window : public CompositeWidget
{ 
protected:
    bool Dragging;
    const char* windowTitle;
    
public:
    Window(Widget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h);
    ~Window();
    uint16_t z;
    
    void Draw(GraphicsContext* gc);
    void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);
    void setWindowTitle(const char* title);
    
};

#endif // WINDOW_H