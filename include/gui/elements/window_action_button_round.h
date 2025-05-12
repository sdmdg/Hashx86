#ifndef ACR_BUTTON_H
#define ACR_BUTTON_H

#include <gui/elements/window_action_button.h>

class ACRButton : public ACButton {
public:
    ACRButton(Widget* parent, int32_t x, int32_t y, const char* label);
    ~ACRButton();

    void RedrawToCache() override;
private:

};

#endif // ACR_BUTTON_H
