#ifndef INFO_DIALOG_H
#define INFO_DIALOG_H

#include <gui/bmp.h>
#include <gui/elements/window_action_button.h>
#include <gui/label.h>
#include <gui/window.h>

class InfoDialog : public Window {
private:
    Label* summaryLabel;
    Label* infoLabel;
    Label* detailsLabel;
    ACButton* okButton;
    Bitmap* iconBitmap;

public:
    InfoDialog(CompositeWidget* parent, int32_t width = 540, int32_t height = 220);
    ~InfoDialog();

    void SetTitleText(const char* title);
    void SetIconBitmap(const char* bitmapPath);
    void SetContent(const char* summary, const char* info, const char* details);
    void ShowDialog();
    void HideDialog();

    void Draw(GraphicsDriver* gc) override;
};

#endif  // INFO_DIALOG_H
