#ifndef BUTTON_H
#define BUTTON_H

#include <gui/widget.h>

class Button : public Widget {
public:
    Button(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char* label);
    ~Button();
    void update();
    
    void SetLabel(const char* label);
    void SetWidth(int32_t w);
    void SetHeight(int32_t h);
    
    void RedrawToCache() override;
    void Draw(GraphicsDriver* gc);
    
    // Mouse event handlers
    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;
    
protected:
    char* label;
    bool isPressed;
};

#endif // BUTTON_H
