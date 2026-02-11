#ifndef TASKBAR_H
#define TASKBAR_H

#include <core/elf.h>
#include <core/filesystem/FAT32.h>
#include <core/globals.h>
#include <gui/label.h>
#include <gui/widget.h>

// Layout Constants
#define TASKBAR_HEIGHT 44
#define TASKBAR_PADDING 6

// Start Button
#define START_BUTTON_WIDTH 42
#define START_BUTTON_HEIGHT 34

// Start Menu
#define START_MENU_WIDTH 240
#define START_MENU_ITEM_HEIGHT 36
#define START_MENU_HEADER_HEIGHT 40
#define START_MENU_PADDING 6
#define START_MENU_MAX_ITEMS 8

// Taskbar Tabs
#define TASKBAR_TAB_HEIGHT 28
#define TASKBAR_TAB_MAX_WIDTH 160
#define TASKBAR_TAB_MIN_WIDTH 60
#define TASKBAR_TAB_PADDING 4
#define TASKBAR_TAB_MAX_TABS 10

// Clock
#define TASKBAR_CLOCK_WIDTH 70

// Color Palette
// Taskbar
#define TASKBAR_BG_COLOR 0xFF1E1E1E
#define TASKBAR_BG_COLOR_TOP 0xFF2A2A2A
#define TASKBAR_BORDER_COLOR 0xFF3A3A3A
#define TASKBAR_SEPARATOR_COLOR 0xFF3A3A3A

// Start Button
#define START_BTN_BG_NORMAL 0xFF2D2D2D
#define START_BTN_BG_HOVER 0xFF383838
#define START_BTN_BG_PRESSED 0xFF252525
#define START_BTN_BG_ACTIVE 0xFF0078D4
#define START_BTN_BORDER 0xFF404040

// Start Menu
#define START_MENU_BG 0xFF252525
#define START_MENU_BORDER 0xFF404040
#define START_MENU_HEADER_BG 0xFF1E1E1E
#define START_MENU_HEADER_TEXT 0xFF8A8A8A
#define START_MENU_ITEM_BG_NORMAL 0x00000000
#define START_MENU_ITEM_BG_HOVER 0xFF353535
#define START_MENU_ITEM_BG_PRESSED 0xFF2A2A2A
#define START_MENU_ITEM_TEXT 0xFFE0E0E0
#define START_MENU_ITEM_DESC_TEXT 0xFF888888
#define START_MENU_SEPARATOR 0xFF3A3A3A

// Taskbar Tab Colors
#define TASKBAR_TAB_BG_NORMAL 0xFF2D2D2D
#define TASKBAR_TAB_BG_HOVER 0xFF383838
#define TASKBAR_TAB_BG_ACTIVE 0xFF404040
#define TASKBAR_TAB_BORDER 0xFF4A4A4A
#define TASKBAR_TAB_TEXT_NORMAL 0xFFB0B0B0
#define TASKBAR_TAB_TEXT_ACTIVE 0xFFFFFFFF
#define TASKBAR_TAB_INDICATOR_ACTIVE 0xFF0078D4

// Clock
#define TASKBAR_CLOCK_TEXT 0xFFB0B0B0
#define TASKBAR_CLOCK_BG_HOVER 0xFF353535

// Data Structures
struct TaskbarAppEntry {
    char name[32];
    char binPath[64];
    char description[64];
};

// Widget Classes

/**
 * @class       StartMenuButton
 * @brief       A single entry in the start menu that launches an application.
 */
class StartMenuButton : public Widget {
private:
    char* label;
    char* description;
    char* binPath;
    bool isPressed;
    bool isHovered;

public:
    StartMenuButton(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* label,
                    const char* description, const char* binPath);
    ~StartMenuButton();

    void RedrawToCache() override;
    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;

    void LaunchProgram();
};

/**
 * @class       TaskbarTab
 * @brief       A tab in the taskbar representing a running process with a GUI window.
 */
class TaskbarTab : public Widget {
private:
    char* label;
    uint32_t pid;
    Widget* windowWidget;  // The window this tab represents
    bool isHovered;
    bool isActive;

public:
    TaskbarTab(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* label,
               uint32_t pid, Widget* window);
    ~TaskbarTab();

    uint32_t GetPID() const {
        return pid;
    }
    Widget* GetWindow() const {
        return windowWidget;
    }
    void SetActive(bool active);
    bool IsActive() const {
        return isActive;
    }
    const char* GetLabel() const {
        return label;
    }

    void RedrawToCache() override;
    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;
};

/**
 * @class       StartMenu
 * @brief       Popup menu with application entries, opened by the Start button.
 */
class StartMenu : public CompositeWidget {
private:
    int32_t itemCount;

public:
    StartMenu(CompositeWidget* parent, int32_t x, int32_t y, int32_t w, int32_t h);
    ~StartMenu();

    void AddApp(const char* name, const char* description, const char* binPath);
    void Draw(GraphicsDriver* gc) override;
    void RedrawToCache() override;
};

/**
 * @class       StartButton
 * @brief       The main start button on the taskbar that toggles the StartMenu.
 */
class StartButton : public Widget {
private:
    bool isPressed;
    bool isActive;  // Menu is open

public:
    StartButton(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h);
    ~StartButton();

    void SetActive(bool active);
    bool IsActive() const {
        return isActive;
    }

    void RedrawToCache() override;
    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;
};

/**
 * @class       Taskbar
 * @brief       Bottom-docked taskbar with Start button, clock, and start menu.
 */
class Taskbar : public CompositeWidget {
private:
    StartButton* startButton;
    StartMenu* startMenu;
    Label* clockLabel;
    uint32_t lastUpdateTick;
    LinkedList<TaskbarTab*> tabs;
    int32_t tabCount = 0;

    void UpdateClock();

public:
    Taskbar(CompositeWidget* parent, int32_t screenW, int32_t screenH);
    ~Taskbar();

    void AddApp(const char* name, const char* description, const char* binPath);
    void ToggleStartMenu();
    void CloseStartMenu();
    bool IsStartMenuOpen() const;
    bool StartMenuContains(int32_t screenX, int32_t screenY) const;

    // Tab management
    void AddTab(uint32_t pid, const char* title, Widget* window);
    void RemoveTabByPID(uint32_t pid);
    void SetActiveTab(Widget* window);
    void RepositionTabs();

    void Draw(GraphicsDriver* gc) override;
    void RedrawToCache() override;

    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;

    // Access for Desktop to draw the menu
    StartMenu* GetStartMenu() {
        return startMenu;
    }
};

#endif  // TASKBAR_H
