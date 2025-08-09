#include <Hx86/Hgui/messagebox.h>

MessageBox::MessageBox(void* parent, const char* title, const char* message, Type type, int* resultPtr)
:Window(parent, 0, 0, 0, 0), message(message), resultPtr(resultPtr) {
    this->setWindowTitle(title);
}

MessageBox::~MessageBox() {
}