/**
 * @file        messagebox.cpp
 * @brief       MessageBox Component (part of #x86 GUI Framework)
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <gui/messagebox.h>

MessageBox::MessageBox(CompositeWidget* parent, const char* title, const char* message, Type type,
                       int* resultPtr)
    : Window(parent, 0, 0, MSGBOXWIDTH, MSGBOXHEIGHT), message(message), resultPtr(resultPtr) {
    this->setWindowTitle(title);
    this->x = (int32_t)(GUI_SCREEN_WIDTH - MSGBOXWIDTH) / 2;
    this->y = (int32_t)(GUI_SCREEN_HEIGHT - MSGBOXHEIGHT) / 2;

    if (type == INFO) {
        okButton = new ACButton(this, w - 40, h - 35, 60, 25, "OK");
        okButton->OnClick(this,
                          [](void* instance) { static_cast<Window*>(instance)->OnCloseButton(); });
        this->AddChild(okButton);
    } /* else if (type == YES_NO) {
        yesButton = new Button(this, w / 2 - 70, h - 40, 60, 25, "Yes");
        noButton = new Button(this, w / 2 + 10, h - 40, 60, 25, "No");

        yesButton->OnClick(this, [](void* instance) {
            MessageBox* box = static_cast<MessageBox*>(instance);
            if (box->resultPtr) *box->resultPtr = 1;
            box->parent->RemoveChild(box);
            delete box;
        });

        noButton->OnClick(this, [](void* instance) {
            MessageBox* box = static_cast<MessageBox*>(instance);
            if (box->resultPtr) *box->resultPtr = 0;
            box->parent->RemoveChild(box);
            delete box;
        });

        this->AddChild(yesButton);
        this->AddChild(noButton);
    }*/
}

MessageBox::~MessageBox() {
    delete okButton;
    delete yesButton;
    delete noButton;
}

void MessageBox::Draw(GraphicsDriver* gc) {
    Window::Draw(gc);
    int X = 0, Y = 0;
    ModelToScreen(X, Y);
    gc->DrawString(X + 20, Y + 40, message, this->font, WINDOW_TITLE_COLOR);
}
