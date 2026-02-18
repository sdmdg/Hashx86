#ifndef WINDOW_H
#define WINDOW_H

#include <core/scheduler.h>
#include <gui/desktop.h>
#include <gui/elements/window_action_button_round.h>
#include <gui/widget.h>

/**
 * @class       Window
 * @brief       A draggable container window with a title bar and close button.
 */
class Window : public CompositeWidget {
protected:
    bool isDragging;
    char* windowTitle;
    ACRButton* closeButton;  // Assuming ACRButton is the specific type

public:
    Window(CompositeWidget* parent, int32_t x, int32_t y, int32_t w, int32_t h);
    ~Window();

    // Core Lifecycle
    void OnClose();
    void setVisible(bool val);
    void setWindowTitle(const char* title);
    const char* getWindowTitle() const {
        return windowTitle;
    }

    // Drawing
    void Draw(GraphicsDriver* gc) override;
    void RedrawToCache() override;

    // Events
    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;
};

#endif  // WINDOW_H
