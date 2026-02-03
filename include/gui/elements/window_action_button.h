#ifndef AC_BUTTON_H
#define AC_BUTTON_H

#include <gui/button.h>

class ACButton : public Button {
public:
    ACButton(Widget* parent, int32_t x, int32_t y, const char* label);
    virtual ~ACButton();

    // Mouse event handlers
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;

    // Callbacks
    void OnClick(void (*callback)());
    void OnClick(void* instance, void (*callback)(void*));

protected:
    // Function pointer for non-member callback
    void (*onClickPtr)();

    // Member function pointer handling
    void* callbackInstance;
    void (*onClickMemberPtr)(void*);
};

#endif  // AC_BUTTON_H
