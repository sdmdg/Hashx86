#ifndef AC_BUTTON_H
#define AC_BUTTON_H

#include <gui/button.h>

class ACButton : public Button {
public:
    ACButton(Widget* parent, int32_t x, int32_t y, const char* label);
    ~ACButton();

    // Mouse event handlers
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;

    void OnClick(void (*callback)());
    void OnClick(void* instance, void (*callback)(void*));

private:
    // Function pointer for non-member callback
    void (*onClickPtr)();  

    // Member function pointer handling
    void* callbackInstance;
    void (*onClickMemberPtr)(void*);
};

#endif // BUTTON_H
