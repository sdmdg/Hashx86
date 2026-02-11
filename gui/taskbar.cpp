/**
 * @file        taskbar.cpp
 * @brief       Taskbar with Start Menu (part of #x86 GUI Framework)
 *
 * @date        11/02/2026
 * @version     2.0.0
 */

#include <core/timing.h>
#include <gui/taskbar.h>

// StartMenuButton
StartMenuButton::StartMenuButton(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h,
                                 const char* label, const char* description, const char* binPath)
    : Widget(parent, x, y, w, h), isPressed(false), isHovered(false) {
    this->font = FontManager::activeInstance->getNewFont();
    this->font->setSize(SMALL);

    this->label = new char[strlen(label) + 1];
    strcpy(this->label, label);

    this->description = new char[strlen(description) + 1];
    strcpy(this->description, description);

    this->binPath = new char[strlen(binPath) + 1];
    strcpy(this->binPath, binPath);
}

StartMenuButton::~StartMenuButton() {
    if (label) delete[] label;
    if (description) delete[] description;
    if (binPath) delete[] binPath;
    if (font) delete font;
}

void StartMenuButton::RedrawToCache() {
    memset(cache, 0, sizeof(uint32_t) * w * h);

    uint32_t bgColor = START_MENU_ITEM_BG_NORMAL;
    if (isPressed) {
        bgColor = START_MENU_ITEM_BG_PRESSED;
    } else if (isHovered) {
        bgColor = START_MENU_ITEM_BG_HOVER;
    }

    // Only draw background if non-transparent
    if ((bgColor >> 24) != 0) {
        NINA::activeInstance->FillRoundedRectangle(cache, w, h, 4, 0, w - 8, h, 4, bgColor);
    }

    // App icon placeholder(small colored dot)
    int32_t iconX = 14;
    int32_t iconY = (h - 8) / 2;
    NINA::activeInstance->FillCircle(cache, w, h, iconX + 4, iconY + 4, 4, 0xFF0078D4);

    // App name
    int32_t textX = 30;
    int32_t textY = 4;
    NINA::activeInstance->DrawString(cache, w, h, textX, textY, label, font, START_MENU_ITEM_TEXT);

    // Description
    if (strlen(description) > 0) {
        Font* descFont = FontManager::activeInstance->getNewFont();
        descFont->setSize(TINY);
        int32_t descY = textY + font->getLineHeight() + 1;
        NINA::activeInstance->DrawString(cache, w, h, textX, descY, description, descFont,
                                         START_MENU_ITEM_DESC_TEXT);
        delete descFont;
    }

    isDirty = false;
}

void StartMenuButton::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    if (!isVisible) return;
    isPressed = true;
    MarkDirty();
}

void StartMenuButton::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    if (!isVisible) return;
    if (isPressed) {
        isPressed = false;
        isHovered = false;
        MarkDirty();
        LaunchProgram();
    }
}

void StartMenuButton::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {
    bool inside = this->ContainsCoordinate(newx, newy);
    bool wasInside = this->ContainsCoordinate(oldx, oldy);

    if (inside && !isHovered) {
        isHovered = true;
        MarkDirty();
    } else if (!inside && isHovered) {
        isHovered = false;
        isPressed = false;
        MarkDirty();
    }
}

void StartMenuButton::LaunchProgram() {
    if (!g_bootPartition || !g_elfLoader) return;

    File* file = g_bootPartition->Open((char*)binPath);
    if (file && file->size > 0) {
        ProgramArguments* args = new ProgramArguments{"ARG1", "ARG2", "ARG3", "ARG4", "ARG5"};
        if (args) {
            ProcessControlBlock* prog = g_elfLoader->loadELF(file, args);
            if (!prog) {
                DEBUG_LOG("StartMenu: Failed to load ELF: %s\n", binPath);
            }
        }
        file->Close();
        delete file;
    } else {
        DEBUG_LOG("StartMenu: File not found: %s\n", binPath);
        if (file) {
            file->Close();
            delete file;
        }
    }
}

// StartMenu
StartMenu::StartMenu(CompositeWidget* parent, int32_t x, int32_t y, int32_t w, int32_t h)
    : CompositeWidget(parent, x, y, w, h), itemCount(0) {
    this->isFocussable = false;
    this->isVisible = false;  // Hidden by default
}

StartMenu::~StartMenu() {}

void StartMenu::AddApp(const char* name, const char* description, const char* binPath) {
    if (itemCount >= START_MENU_MAX_ITEMS) return;

    int32_t itemY =
        START_MENU_HEADER_HEIGHT + START_MENU_PADDING + itemCount * (START_MENU_ITEM_HEIGHT + 2);

    StartMenuButton* btn = new StartMenuButton(this, 0, itemY, this->w, START_MENU_ITEM_HEIGHT,
                                               name, description, binPath);

    this->AddChild(btn);
    itemCount++;

    // Resize menu height to fit content
    int32_t totalH = START_MENU_HEADER_HEIGHT + START_MENU_PADDING +
                     itemCount * (START_MENU_ITEM_HEIGHT + 2) + START_MENU_PADDING;
    this->h = totalH;

    // Reallocate cache for new size
    if (this->cache) delete[] this->cache;
    if (this->w > 0 && this->h > 0) {
        this->cache = new uint32_t[this->w * this->h]();
    }
}

void StartMenu::Draw(GraphicsDriver* gc) {
    if (!isVisible) return;

    // Check children dirty
    for (auto& child : childrenList) {
        if (child->isDirty) this->isDirty = true;
    }

    if (isDirty) {
        RedrawToCache();
        isDirty = false;
    }

    int X = 0, Y = 0;
    ModelToScreen(X, Y);
    gc->DrawBitmap(X, Y, (const uint32_t*)cache, w, h);
}

void StartMenu::RedrawToCache() {
    memset(cache, 0, sizeof(uint32_t) * w * h);

    // Background with rounded corners
    NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, w, h, 8, START_MENU_BG);

    // Border
    NINA::activeInstance->DrawRoundedRectangle(cache, w, h, 0, 0, w, h, 8, START_MENU_BORDER);

    // Header area
    Font* headerFont = FontManager::activeInstance->getNewFont();
    headerFont->setSize(SMALL);
    NINA::activeInstance->DrawString(cache, w, h, 14, 12, "Applications", headerFont,
                                     START_MENU_HEADER_TEXT);
    delete headerFont;

    // Separator line under header
    NINA::activeInstance->DrawHorizontalLine(cache, w, h, 10, START_MENU_HEADER_HEIGHT - 2, w - 20,
                                             START_MENU_SEPARATOR);

    // Composite children (menu items) onto cache
    for (auto& child : childrenList) {
        if (!child->isVisible) continue;
        if (child->isDirty) child->RedrawToCache();

        NINA::activeInstance->DrawBitmapToBuffer(cache, w, h, child->x, child->y, child->cache,
                                                 child->w, child->h);
    }
}

// StartButton
StartButton::StartButton(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h)
    : Widget(parent, x, y, w, h), isPressed(false), isActive(false) {
    this->font = FontManager::activeInstance->getNewFont();
    this->font->setSize(SMALL);
}

StartButton::~StartButton() {
    if (font) delete font;
}

void StartButton::SetActive(bool active) {
    if (isActive != active) {
        isActive = active;
        MarkDirty();
    }
}

void StartButton::RedrawToCache() {
    memset(cache, 0, sizeof(uint32_t) * w * h);

    uint32_t bgColor;
    if (isActive) {
        bgColor = START_BTN_BG_ACTIVE;
    } else if (isPressed) {
        bgColor = START_BTN_BG_PRESSED;
    } else {
        bgColor = START_BTN_BG_NORMAL;
    }

    // Circle background
    int32_t radius = (w < h ? w : h) / 2;
    int32_t cx = w / 2;
    int32_t cy = h / 2;
    NINA::activeInstance->FillCircle(cache, w, h, cx, cy, radius, bgColor);

    if (!isActive) {
        NINA::activeInstance->DrawCircle(cache, w, h, cx, cy, radius, START_BTN_BORDER);
    }

    // Draw the OS icon (icon_main_20x20) centered
    int32_t iconX = (w - 20) / 2;
    int32_t iconY = (h - 20) / 2;
    NINA::activeInstance->DrawBitmap(cache, w, h, iconX, iconY, (const uint32_t*)icon_main_20x20,
                                     20, 20);

    isDirty = false;
}

void StartButton::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    if (!isVisible) return;
    isPressed = true;
    MarkDirty();
}

void StartButton::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    if (!isVisible) return;
    if (isPressed) {
        isPressed = false;
        MarkDirty();

        // Tell the parent Taskbar to toggle the menu
        Taskbar* tb = (Taskbar*)this->parent;
        if (tb) tb->ToggleStartMenu();
    }
}

void StartButton::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {
    bool inside = this->ContainsCoordinate(newx, newy);
    if (isPressed && !inside) {
        isPressed = false;
        MarkDirty();
    }
}

// TaskbarTab
TaskbarTab::TaskbarTab(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h,
                       const char* label, uint32_t pid, Widget* window)
    : Widget(parent, x, y, w, h),
      pid(pid),
      windowWidget(window),
      isHovered(false),
      isActive(false) {
    this->font = FontManager::activeInstance->getNewFont();
    this->font->setSize(TINY);

    this->label = new char[strlen(label) + 1];
    if (!this->label) {
        HALT("CRITICAL: Failed to allocate tab label!\n");
    }
    strcpy(this->label, label);
}

TaskbarTab::~TaskbarTab() {
    if (label) delete[] label;
    if (font) delete font;
}

void TaskbarTab::SetActive(bool active) {
    if (isActive != active) {
        isActive = active;
        MarkDirty();
    }
}

void TaskbarTab::RedrawToCache() {
    memset(cache, 0, sizeof(uint32_t) * w * h);

    uint32_t bgColor = TASKBAR_TAB_BG_NORMAL;
    uint32_t textColor = TASKBAR_TAB_TEXT_NORMAL;

    if (isActive) {
        bgColor = TASKBAR_TAB_BG_ACTIVE;
        textColor = TASKBAR_TAB_TEXT_ACTIVE;
    } else if (isHovered) {
        bgColor = TASKBAR_TAB_BG_HOVER;
    }

    // Tab background
    NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, w, h, 4, bgColor);

    // Active indicator line at the bottom
    if (isActive) {
        NINA::activeInstance->FillRectangle(cache, w, h, 4, h - 3, w - 8, 2,
                                            TASKBAR_TAB_INDICATOR_ACTIVE);
    }

    // Tab title text (truncated to fit)
    int32_t textX = 8;
    int32_t textY = (h - font->getLineHeight()) / 2;
    NINA::activeInstance->DrawString(cache, w, h, textX, textY, label, font, textColor);

    isDirty = false;
}

void TaskbarTab::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    if (!isVisible) return;

    // Bring the associated window to the front
    if (windowWidget && windowWidget->parent) {
        windowWidget->parent->GetFocus(windowWidget);
    }

    // Set this tab as active
    Taskbar* tb = (Taskbar*)this->parent;
    if (tb) {
        tb->SetActiveTab(windowWidget);
    }
}

void TaskbarTab::OnMouseUp(int32_t x, int32_t y, uint8_t button) {}

void TaskbarTab::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {
    bool inside = this->ContainsCoordinate(newx, newy);
    bool wasInside = this->ContainsCoordinate(oldx, oldy);

    if (inside && !isHovered) {
        isHovered = true;
        MarkDirty();
    } else if (!inside && isHovered) {
        isHovered = false;
        MarkDirty();
    }
}

// Taskbar tab management
void Taskbar::AddTab(uint32_t pid, const char* title, Widget* window) {
    if (tabCount >= TASKBAR_TAB_MAX_TABS) return;

    // Don't add duplicate tabs for the same PID
    bool exists = false;
    tabs.ForEach([&](TaskbarTab* t) {
        if (t->GetPID() == pid) exists = true;
    });
    if (exists) return;

    int32_t tabY = (TASKBAR_HEIGHT - TASKBAR_TAB_HEIGHT) / 2;
    TaskbarTab* tab = new TaskbarTab(this, 0, tabY, TASKBAR_TAB_MAX_WIDTH, TASKBAR_TAB_HEIGHT,
                                     title, pid, window);
    if (!tab) {
        HALT("CRITICAL: Failed to allocate TaskbarTab!\n");
    }

    tabs.Add(tab);
    this->AddChild(tab);
    tabCount++;

    RepositionTabs();

    // Set the new tab as active
    SetActiveTab(window);
}

void Taskbar::RemoveTabByPID(uint32_t pid) {
    TaskbarTab* found = nullptr;
    tabs.ForEach([&](TaskbarTab* t) {
        if (!found && t->GetPID() == pid) found = t;
    });

    if (found) {
        tabs.Remove([&](TaskbarTab* t) { return t == found; });
        this->RemoveChild(found);
        delete found;
        tabCount--;
        RepositionTabs();
    }
}

void Taskbar::SetActiveTab(Widget* window) {
    tabs.ForEach([&](TaskbarTab* t) { t->SetActive(t->GetWindow() == window); });
    MarkDirty();
}

void Taskbar::RepositionTabs() {
    // Calculate available space for tabs
    int32_t tabAreaStart = TASKBAR_PADDING + START_BUTTON_WIDTH + TASKBAR_PADDING + TASKBAR_PADDING;
    int32_t tabAreaEnd = w - TASKBAR_CLOCK_WIDTH - TASKBAR_PADDING * 3;
    int32_t availableWidth = tabAreaEnd - tabAreaStart;

    if (tabCount <= 0 || availableWidth <= 0) return;

    // Calculate tab width
    int32_t tabWidth = (availableWidth - (tabCount - 1) * TASKBAR_TAB_PADDING) / tabCount;
    if (tabWidth > TASKBAR_TAB_MAX_WIDTH) tabWidth = TASKBAR_TAB_MAX_WIDTH;
    if (tabWidth < TASKBAR_TAB_MIN_WIDTH) tabWidth = TASKBAR_TAB_MIN_WIDTH;

    int32_t currentX = tabAreaStart;
    tabs.ForEach([&](TaskbarTab* tab) {
        tab->x = currentX;
        tab->w = tabWidth;

        // Reallocate cache for new size
        if (tab->cache) delete[] tab->cache;
        tab->cache = new uint32_t[tab->w * tab->h]();

        tab->MarkDirty();
        currentX += tabWidth + TASKBAR_TAB_PADDING;
    });

    MarkDirty();
}

// Taskbar
Taskbar::Taskbar(CompositeWidget* parent, int32_t screenW, int32_t screenH)
    : CompositeWidget(parent, 0, screenH - TASKBAR_HEIGHT, screenW, TASKBAR_HEIGHT) {
    this->lastUpdateTick = 0;
    this->isFocussable = false;

    // Start Button (left)
    int32_t startBtnX = TASKBAR_PADDING;
    int32_t startBtnY = (TASKBAR_HEIGHT - START_BUTTON_HEIGHT) / 2;
    startButton =
        new StartButton(this, startBtnX, startBtnY, START_BUTTON_WIDTH, START_BUTTON_HEIGHT);
    this->AddChild(startButton);

    // Clock Label (right)
    int32_t clockX = screenW - TASKBAR_CLOCK_WIDTH - TASKBAR_PADDING;
    int32_t clockY = (TASKBAR_HEIGHT - 20) / 2;
    clockLabel = new Label(this, clockX, clockY, TASKBAR_CLOCK_WIDTH, 20, "12:00 AM");
    clockLabel->setSize(SMALL);
    this->AddChild(clockLabel);

    // Start Menu (positioned above the taskbar)
    int32_t menuH = START_MENU_HEADER_HEIGHT + START_MENU_PADDING * 2;
    int32_t menuX = TASKBAR_PADDING;
    int32_t menuY = -(menuH);
    startMenu = new StartMenu(this, menuX, menuY, START_MENU_WIDTH, menuH);
}

Taskbar::~Taskbar() {
    if (startMenu) delete startMenu;
}

void Taskbar::AddApp(const char* name, const char* description, const char* binPath) {
    startMenu->AddApp(name, description, binPath);

    // Reposition menu Y to sit just above the taskbar
    startMenu->y = -(startMenu->h);
}

void Taskbar::ToggleStartMenu() {
    if (startMenu->isVisible) {
        CloseStartMenu();
    } else {
        startMenu->isVisible = true;
        startMenu->MarkDirty();
        startButton->SetActive(true);
        if (this->parent) this->parent->MarkDirty();
    }
}

void Taskbar::CloseStartMenu() {
    if (startMenu->isVisible) {
        startMenu->isVisible = false;
        startButton->SetActive(false);
        if (this->parent) this->parent->MarkDirty();
    }
}

bool Taskbar::IsStartMenuOpen() const {
    return startMenu && startMenu->isVisible;
}

bool Taskbar::StartMenuContains(int32_t screenX, int32_t screenY) const {
    if (!startMenu || !startMenu->isVisible) return false;

    // The menu is at taskbar.x + menu.x, taskbar.y + menu.y (in screen coords)
    int32_t menuScreenX = this->x + startMenu->x;
    int32_t menuScreenY = this->y + startMenu->y;

    return (screenX >= menuScreenX && screenX < menuScreenX + startMenu->w &&
            screenY >= menuScreenY && screenY < menuScreenY + startMenu->h);
}

// Read a CMOS/RTC register
static uint8_t rtc_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

// Convert BCD to binary
static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void Taskbar::UpdateClock() {
    // Wait until RTC update is not in progress
    while (rtc_read(0x0A) & 0x80);

    // [uint8_t seconds = rtc_read(0x00);]
    uint8_t minutes = rtc_read(0x02);
    uint8_t hours = rtc_read(0x04);

    // Check if RTC is in BCD mode (bit 2 of status register B = 0 means BCD)
    uint8_t regB = rtc_read(0x0B);
    bool is24Hour = regB & 0x02;
    bool isPM = false;

    if (!is24Hour) {
        // 12-hour mode: bit 7 of hours is PM flag
        isPM = hours & 0x80;
        hours &= 0x7F;  // strip PM bit before BCD conversion
    }

    if (!(regB & 0x04)) {
        // seconds = bcd_to_bin(seconds);
        minutes = bcd_to_bin(minutes);
        hours = bcd_to_bin(hours);
    }

    // Convert 12-hour RTC to 24-hour for timezone math
    if (!is24Hour) {
        if (isPM && hours != 12) hours += 12;
        if (!isPM && hours == 12) hours = 0;
    }

    // Apply timezone offset
    int32_t totalMinutes = (int32_t)hours * 60 + (int32_t)minutes;
    totalMinutes += TIMEZONE_HOURS * 60 + TIMEZONE_MINUTES;
    while (totalMinutes < 0) totalMinutes += 1440;
    totalMinutes %= 1440;
    hours = (uint8_t)(totalMinutes / 60);
    minutes = (uint8_t)(totalMinutes % 60);

    // Convert to 12-hour format with AM/PM
    const char* ampm = "AM";
    if (hours >= 12) {
        ampm = "PM";
        if (hours > 12) hours -= 12;
    }
    if (hours == 0) hours = 12;

    char timeStr[9];
    timeStr[0] = '0' + (hours / 10);
    timeStr[1] = '0' + (hours % 10);
    timeStr[2] = ':';
    timeStr[3] = '0' + (minutes / 10);
    timeStr[4] = '0' + (minutes % 10);
    timeStr[5] = ' ';
    timeStr[6] = ampm[0];
    timeStr[7] = ampm[1];
    timeStr[8] = '\0';

    clockLabel->setText(timeStr);
}

void Taskbar::Draw(GraphicsDriver* gc) {
    // Clock update
    uint32_t currentTick = (uint32_t)timerTicks;
    if (currentTick - lastUpdateTick >= 1000) {
        UpdateClock();
        lastUpdateTick = currentTick;
    }

    // Check children dirty
    for (auto& child : childrenList) {
        if (child->isDirty) this->isDirty = true;
    }

    if (isDirty) {
        if (isVisible) {
            RedrawToCache();
        } else {
            memset(cache, 0, sizeof(uint32_t) * w * h);
        }
        isDirty = false;
    }

    if (isVisible) {
        int X = 0, Y = 0;
        ModelToScreen(X, Y);
        gc->DrawBitmap(X, Y, (const uint32_t*)cache, w, h);
    }

    // Draw start menu ON TOP
    if (startMenu && startMenu->isVisible) {
        // Check if menu is dirty
        if (startMenu->isDirty) {
            startMenu->RedrawToCache();
            startMenu->isDirty = false;
        }

        int X = 0, Y = 0;
        startMenu->ModelToScreen(X, Y);
        gc->DrawBitmap(X, Y, (const uint32_t*)startMenu->cache, startMenu->w, startMenu->h);
    }
}

void Taskbar::RedrawToCache() {
    memset(cache, 0, sizeof(uint32_t) * w * h);

    // Background
    NINA::activeInstance->FillRectangle(cache, w, h, 0, 0, w, 1, TASKBAR_BG_COLOR_TOP);
    NINA::activeInstance->FillRectangle(cache, w, h, 0, 1, w, h - 1, TASKBAR_BG_COLOR);

    // Top border
    NINA::activeInstance->DrawHorizontalLine(cache, w, h, 0, 0, w, TASKBAR_BORDER_COLOR);

    // Separator after start button
    int32_t sepX = TASKBAR_PADDING + START_BUTTON_WIDTH + TASKBAR_PADDING;
    NINA::activeInstance->DrawVerticalLine(cache, w, h, sepX, 8, TASKBAR_HEIGHT - 16,
                                           TASKBAR_SEPARATOR_COLOR);

    // Separator before clock
    int32_t clockSepX = w - TASKBAR_CLOCK_WIDTH - TASKBAR_PADDING * 2;
    NINA::activeInstance->DrawVerticalLine(cache, w, h, clockSepX, 8, TASKBAR_HEIGHT - 16,
                                           TASKBAR_SEPARATOR_COLOR);

    // Composite children (start button + clock) onto cache
    for (auto& child : childrenList) {
        if (!child->isVisible) continue;
        if (child->isDirty) child->RedrawToCache();

        NINA::activeInstance->DrawBitmapToBuffer(cache, w, h, child->x, child->y, child->cache,
                                                 child->w, child->h);
    }
}

void Taskbar::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    // Check start menu first
    if (startMenu && startMenu->isVisible) {
        int32_t menuLocalX = localX - startMenu->x;
        int32_t menuLocalY = localY - startMenu->y;
        if (menuLocalX >= 0 && menuLocalX < startMenu->w && menuLocalY >= 0 &&
            menuLocalY < startMenu->h) {
            startMenu->OnMouseDown(localX, localY, button);
            return;
        }
    }

    // clicked children in the taskbar
    Widget* clicked = nullptr;
    childrenList.ReverseForEach([&](Widget* child) {
        if (!clicked && child->ContainsCoordinate(localX, localY)) {
            child->OnMouseDown(localX, localY, button);
            clicked = child;
        }
    });
}

void Taskbar::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
    int32_t localX = x - this->x;
    int32_t localY = y - this->y;

    // Route to start menu if visible
    if (startMenu && startMenu->isVisible) {
        int32_t menuLocalX = localX - startMenu->x;
        int32_t menuLocalY = localY - startMenu->y;
        if (menuLocalX >= 0 && menuLocalX < startMenu->w && menuLocalY >= 0 &&
            menuLocalY < startMenu->h) {
            startMenu->OnMouseUp(localX, localY, button);
            // Close menu after launching
            CloseStartMenu();
            return;
        }
    }

    childrenList.ForEach([&](Widget* child) {
        if (child->ContainsCoordinate(localX, localY)) {
            child->OnMouseUp(localX, localY, button);
        }
    });
}

void Taskbar::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {
    int32_t localOldX = oldx - this->x;
    int32_t localOldY = oldy - this->y;
    int32_t localNewX = newx - this->x;
    int32_t localNewY = newy - this->y;

    // Route to start menu
    if (startMenu && startMenu->isVisible) {
        startMenu->OnMouseMove(localOldX, localOldY, localNewX, localNewY);
    }

    // Route to children
    childrenList.ForEach([&](Widget* child) {
        bool inOld = child->ContainsCoordinate(localOldX, localOldY);
        bool inNew = child->ContainsCoordinate(localNewX, localNewY);
        if (inOld || inNew) {
            child->OnMouseMove(localOldX, localOldY, localNewX, localNewY);
        }
    });
}
