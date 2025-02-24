#ifndef WIDGET_H
#define WIDGET_H

#include <types.h>
#include <gui/graphicscontext.h>
#include <gui/fonts/font.h>
#include <gui/fonts/segoeui.h>
#include <gui/icons.h>
#include <core/drivers/keyboard.h>
#include <gui/config/config.h>

const SegoeUI mainfont;

class Widget : public KeyboardEventHandler
{
protected:
    Widget* parent;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    Font* font  = (SegoeUI*) &mainfont;
 
    uint32_t colorIndex;
    bool Focussable;
    uint32_t GUI_SCREEN_WIDTH = 1024;
    uint32_t GUI_SCREEN_HEIGHT = 768;

public:

    Widget(Widget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h);
    ~Widget();
    
    virtual void GetFocus(Widget* widget);
    virtual void ModelToScreen(int32_t &x, int32_t& y);
    virtual bool ContainsCoordinate(int32_t x, int32_t y);
    
    virtual void Draw(GraphicsContext* gc);
    virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);
};


class CompositeWidget : public Widget
{
private:

    Widget* focussedChild;
    
public:
    Widget* children[100];
    int numChildren;
    CompositeWidget(Widget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h);
    ~CompositeWidget();            
    
    virtual void GetFocus(Widget* widget);
    virtual bool AddChild(Widget* child);
    
    virtual void Draw(GraphicsContext* gc);
    virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);
};


#endif // WIDGET_H