#ifndef WIDGET_H
#define WIDGET_H

#include <types.h>
#include <utils/string.h>
#include <gui/graphicscontext.h>
#include <gui/fonts/font.h>
#include <gui/fonts/segoeui.h>
#include <gui/icons.h>
#include <core/drivers/keyboard.h>
#include <core/memory.h>
#include <gui/config/config.h>
#include <gui/renderer/nina.h>
#include <debug.h>
#include <utils/linkedList.h>

class Widget : public KeyboardEventHandler
{
    friend class CompositeWidget;
protected:

    Font* font;

    uint32_t colorIndex;
    bool Focussable;

    

public:
    Widget(Widget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h);
    ~Widget();
    LinkedList<Widget*> childrenList;

    Widget* parent;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    uint32_t* cache;
    bool isDirty = true;
    bool isVisible = true;
    
    uint32_t PID = 0;
    uint32_t ID = 0;
    virtual void MarkDirty();
    virtual void RedrawToCache(); 
    
    virtual void GetFocus(Widget* widget);
    virtual void SetFocussable(bool focussable);
    virtual void SetPID(uint32_t PID);
    virtual void SetID(uint32_t ID);
    virtual Widget* FindWidgetByID(uint32_t searchID);
    virtual Widget* FindWidgetByPID(uint32_t PID);
    virtual bool AddChild(Widget* child);
    virtual bool RemoveChild(Widget* child);

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
    CompositeWidget(CompositeWidget* parent,
            int32_t x, int32_t y, int32_t w, int32_t h);
    ~CompositeWidget();            
    
    virtual void GetFocus(Widget* widget);
    
    virtual void Draw(GraphicsContext* gc);
    virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);
};


#endif // WIDGET_H