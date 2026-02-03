#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <gui/elements/window_action_button.h>
#include <gui/window.h>

enum Type { INFO /* , YES_NO */ };

class MessageBox : public Window {
private:
    ACButton* okButton;
    ACButton* yesButton;
    ACButton* noButton;
    const char* message;
    int* resultPtr;

public:
    MessageBox(CompositeWidget* parent, const char* title, const char* message, Type type,
               int* resultPtr);
    ~MessageBox();
    void Draw(GraphicsDriver* gc) override;
};

#endif  // MESSAGEBOX_H
