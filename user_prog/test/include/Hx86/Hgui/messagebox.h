#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <Hx86/Hgui/window.h>
#include <Hx86/Hgui/button.h>

enum Type { INFO/* , YES_NO */ };

class MessageBox : public Window {
private:
    const char* message;
    int* resultPtr;

public:
    MessageBox(void* parent, const char* title, const char* message, Type type, int* resultPtr);
    ~MessageBox();
};

#endif // MESSAGEBOX_H