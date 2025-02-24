#ifndef BUTTON_H
#define BUTTON_H

#include <gui/widget.h>

class Button : public Widget {
public:
    Button(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* label);
    ~Button();
    
    void SetLabel(const char* label);
    
    // Set a function to be called when the button is clicked
    void OnClick(void (*callback)());
    
    void Draw(GraphicsContext* gc) override;
    
    // Mouse event handlers
    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;
    
private:
    const char* label;
    bool isPressed;
    uint8_t paddingX;
    uint8_t paddingY;
    void (*onClickPtr)();  // Function pointer for click callback
};

#endif // BUTTON_H
