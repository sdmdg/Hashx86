#ifndef HWIDGET_H
#define HWIDGET_H

#include <Hx86/stdint.h>
#include <Hx86/Hsyscalls/Hsyscallsgui.h>
#include <Hx86/utils/linkedList.h>

struct WidgetData {
    uint32_t param0;
    int32_t param1;
    int32_t param2;
    uint32_t param3;
    uint32_t param4;
    const char* param5;
    char* param6;
    char* param7;
};

typedef enum {
    REGULAR = 0x0,
    BOLD    = 0x1,
    ITALIC  = 0x2,
    BOLD_ITALIC  = 0x3,
} FontType;

typedef enum {
    TINY    = 0,
    SMALL   = 1,
    MEDIUM  = 2,
    LARGE   = 3,
    XLARGE  = 4,
} FontSize;





class Widget
{
protected:
    
    int32_t pid;
    LinkedList<Widget*> childrenList;



public:
    Widget(Widget* parent,
            int32_t x, int32_t y, uint32_t w, uint32_t h);
    ~Widget();
    uint32_t ID;
    Widget* parent;
        // Function pointer for non-member callback
        void (*onClickPtr)();  

        // Member function pointer handling
        void* callbackInstance;
        void (*onClickMemberPtr)(void*);

    Widget* FindWidgetByID(uint32_t searchID);
    virtual bool AddChild(Widget* child);
    virtual bool RemoveChild(Widget* child);
    
    virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);


    // Allow both global and member function callbacks
    void OnClick(void (*callback)()); // Non-member function
    void OnClick(void* instance, void (*callback)(void*)); // Member function wrapper
};


class CompositeWidget : public Widget
{
private:
    
public:
    CompositeWidget(Widget* parent,
            int32_t x, int32_t y, uint32_t w, uint32_t h);
    ~CompositeWidget();
};


#endif // HWIDGET_H