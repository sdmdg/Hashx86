/**
 * @file        infodialog.cpp
 * @brief       Generic Info Dialog
 *
 * @date        11/03/2026
 * @version     1.0.0
 */

#define KDBG_COMPONENT "GUI:INFODLG"
#include <gui/infodialog.h>

InfoDialog::InfoDialog(CompositeWidget* parent, int32_t width, int32_t height)
    : Window(parent, 0, 0, width, height),
      summaryLabel(nullptr),
      infoLabel(nullptr),
      detailsLabel(nullptr),
      okButton(nullptr),
      iconBitmap(nullptr) {
    x = (GUI_SCREEN_WIDTH - w) / 2;
    y = (GUI_SCREEN_HEIGHT - h) / 2;
    SetPID(0);  // Kernel/system owned
    setWindowTitle("Info");

    summaryLabel = new Label(this, 74, 36, w - 90, 26, "");
    infoLabel = new Label(this, 74, 58, w - 90, 20, "");
    detailsLabel = new Label(this, 18, 88, w - 36, h - 118, "");
    okButton = new ACButton(this, w - 86, h - 30, "OK");

    if (!summaryLabel || !infoLabel || !detailsLabel || !okButton) {
        HALT("CRITICAL: Failed to allocate info dialog widgets!\n");
    }

    summaryLabel->setSize(MEDIUM);
    infoLabel->setSize(SMALL);
    detailsLabel->setSize(SMALL);

    okButton->SetWidth(64);
    okButton->SetHeight(20);
    okButton->OnClick(this,
                      [](void* instance) { static_cast<InfoDialog*>(instance)->HideDialog(); });

    AddChild(summaryLabel);
    AddChild(infoLabel);
    AddChild(detailsLabel);
    AddChild(okButton);
}

InfoDialog::~InfoDialog() {
    delete summaryLabel;
    delete infoLabel;
    delete detailsLabel;
    delete okButton;
    delete iconBitmap;
}

void InfoDialog::SetTitleText(const char* title) {
    setWindowTitle(title ? title : "Info");
}

void InfoDialog::SetIconBitmap(const char* bitmapPath) {
    if (iconBitmap) {
        delete iconBitmap;
        iconBitmap = nullptr;
    }

    if (!bitmapPath || bitmapPath[0] == '\0') return;

    char* path = (char*)bitmapPath;
    iconBitmap = new Bitmap(path);
    if (iconBitmap && !iconBitmap->IsValid()) {
        delete iconBitmap;
        iconBitmap = nullptr;
    }
}

void InfoDialog::SetContent(const char* summary, const char* info, const char* details) {
    summaryLabel->setText(summary ? summary : "");
    infoLabel->setText(info ? info : "");
    detailsLabel->setText(details ? details : "");
}

void InfoDialog::ShowDialog() {
    x = (GUI_SCREEN_WIDTH - w) / 2;
    y = (GUI_SCREEN_HEIGHT - h) / 2;
    setVisible(true);
    MarkDirty();
}

void InfoDialog::HideDialog() {
    setVisible(false);
    this->parent->RemoveChild(this);
    MarkDirty();
}

void InfoDialog::Draw(GraphicsDriver* gc) {
    Window::Draw(gc);
    if (!isVisible) return;

    int X = 0;
    int Y = 0;
    ModelToScreen(X, Y);

    if (iconBitmap && iconBitmap->IsValid() && iconBitmap->GetWidth() <= 64 &&
        iconBitmap->GetHeight() <= 64) {
        gc->DrawBitmap(X + 16, Y + 38, iconBitmap->GetBuffer(), iconBitmap->GetWidth(),
                       iconBitmap->GetHeight());
    } else {
        gc->DrawBitmap(X + 20, Y + 42, (const uint32_t*)icon_main_20x20, 20, 20);
    }
}
