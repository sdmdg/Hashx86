#ifndef WINDOW_H
#define WINDOW_H

#include <gui/widget.h>
#include <gui/elements/window_action_button_round.h>

class Window : public CompositeWidget
{
protected:
    bool Dragging;
    char* windowTitle;
    ACButton* closeButton;
    
public:
    Window(CompositeWidget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h);
    ~Window();

    
    void update();

    void OnClose();
    
    void Draw(GraphicsContext* gc);
    void RedrawToCache();
    void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);

    void setVisible(bool val);
    void setWindowTitle(const char* title);
    void OnCloseButton();
};

#endif // WINDOW_H