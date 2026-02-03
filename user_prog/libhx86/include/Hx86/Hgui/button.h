#ifndef BUTTON_H
#define BUTTON_H

#include <Hx86/Hgui/widget.h>

class Button : public Widget {
public:
    Button(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char* label);
    ~Button();

    bool setText(const char* label);
    bool setWidth(int32_t w);
    bool setHeight(int32_t h);

    // Mouse event handlers
    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;

private:
    const char* label;
};

#endif  // BUTTON_H
