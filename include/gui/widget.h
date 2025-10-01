#ifndef WIDGET_H
#define WIDGET_H

#include <types.h>
#include <utils/string.h>
#include <gui/graphicscontext.h>
#include <gui/fonts/font.h>
#include <gui/icons.h>
#include <core/drivers/keyboard.h>
#include <core/memory.h>
#include <gui/config/config.h>
#include <gui/renderer/nina.h>
#include <debug.h>
#include <utils/linkedList.h>

class Widget
{
    friend class CompositeWidget;
protected:
    Font* font = nullptr;
    uint32_t colorIndex = 0;
    bool Focussable = true;
    bool isFocussed = false;

public:
    Widget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h);
    virtual ~Widget();

    LinkedList<Widget*> childrenList;
    Widget* parent = nullptr;

    int32_t x = 0, y = 0, w = 0, h = 0;
    uint32_t* cache = nullptr;
    bool isDirty = true;
    bool isVisible = true;

    uint32_t PID = 0;
    uint32_t ID = 0;

    virtual void MarkDirty();
    virtual void RedrawToCache();

    virtual void GetFocus(Widget* widget);
    virtual void setFocus(bool result);
    virtual void SetFocussable(bool focussable);

    virtual void SetPID(uint32_t PID);
    virtual void SetID(uint32_t ID);

    virtual Widget* FindWidgetByID(uint32_t searchID);
    virtual Widget* FindWidgetByPID(uint32_t PID);

    virtual bool AddChild(Widget* child);
    virtual bool RemoveChild(Widget* child);

    virtual void ModelToScreen(int32_t& x, int32_t& y);
    virtual bool ContainsCoordinate(int32_t x, int32_t y);

    virtual void Draw(GraphicsContext* gc);
    virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);

    virtual void OnKeyDown(const char* key);
    virtual void OnSpecialKeyDown(uint8_t key);
    virtual void OnKeyUp(const char* key);
    virtual void OnSpecialKeyUp(uint8_t key);
};


class CompositeWidget : public Widget
{
private:
    Widget* focussedChild = nullptr;

public:
    CompositeWidget(CompositeWidget* parent, int32_t x, int32_t y, int32_t w, int32_t h);
    virtual ~CompositeWidget();

    virtual void GetFocus(Widget* widget);

    virtual void Draw(GraphicsContext* gc);
    virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);

    virtual void OnKeyDown(const char* key);
    virtual void OnSpecialKeyDown(uint8_t key);
    virtual void OnKeyUp(const char* key);
    virtual void OnSpecialKeyUp(uint8_t key);
};

#endif // WIDGET_H
