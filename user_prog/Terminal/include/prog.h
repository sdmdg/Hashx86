#ifndef PROG_H
#define PROG_H

#include <Hx86/Hgui/Hgui.h>
#include <Hx86/Hgui/terminalview.h>
#include <Hx86/Hsyscalls/syscalls.h>
#include <Hx86/Hx86.h>
#include <Hx86/utils/string.h>

namespace {

// --- Constants ---
constexpr uint8_t SC_ESC = 0x01;
constexpr uint8_t SC_BACKSPACE = 0x0E;
constexpr uint8_t SC_ENTER = 0x1C;
constexpr uint8_t SC_LSHIFT = 0x2A;
constexpr uint8_t SC_RSHIFT = 0x36;
constexpr uint8_t SC_CAPSLOCK = 0x3A;
constexpr uint8_t SC_F11 = 0x57;
constexpr uint8_t SC_F12 = 0x58;

constexpr int MAX_LINES = 1200;
constexpr int MAX_COLS = 160;

constexpr int WIN_W = 920;
constexpr int WIN_H = 640;

constexpr int PAD = 10;
constexpr int TOP_Y = 34;

constexpr int TERM_X = PAD;
constexpr int TERM_Y = TOP_Y;
constexpr int TERM_W = WIN_W - (PAD * 2);
constexpr int TERM_H = WIN_H - TOP_Y - 30;

constexpr int STATUS_X = PAD;
constexpr int STATUS_Y = WIN_H - 22;
constexpr int STATUS_W = WIN_W - (PAD * 2);
constexpr int STATUS_H = 18;

constexpr int CHAR_W = 8;
constexpr int CHAR_H = 10;
constexpr int MAX_RENDER_TEXT = 64 * 1024;

// --- Keyboard Maps ---
static const char kNormalMap[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7',  '8', '9', '0',  '-',  '=', 0,   0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',  'p', '[', ']',  '\n', 0,   'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',  'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,    ' ', 0,   0,    0,
};

static const char kShiftMap[128] = {
    0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',  '+', 0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,   'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z',  'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,
};

// --- Inline Helpers ---
inline int max_i(int a, int b) {
    return (a > b) ? a : b;
}
inline int min_i(int a, int b) {
    return (a < b) ? a : b;
}
inline bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
inline char to_upper(char c) {
    if (c >= 'a' && c <= 'z') return (char)(c - 32);
    return c;
}
inline bool string_eq(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return false;
        i++;
    }
    return a[i] == b[i];
}

// --- Structs and Classes ---
struct TerminalBuffer {
    char lines[MAX_LINES][MAX_COLS + 1];
    int lengths[MAX_LINES];
    int lineCount;

    void Init();
    void Clear();
    int CurrentIndex() const;
    void NewLine();
    void AppendChar(char c, int wrapCols);
    void AppendText(const char* text, int wrapCols);
    void Backspace(int protectColumns);
};

class TerminalApp {
public:
    TerminalApp();
    ~TerminalApp();

    bool Init();
    void Run();

private:
    // GUI Widgets
    Window* mainWindow;
    TerminalView* terminalView;
    Label* statusLabel;

    // State
    TerminalBuffer* terminal;
    int scrollOffset;
    bool capsLock;
    int inputStartColumn;
    bool viewDirty;

    // Buffers
    char renderText[MAX_RENDER_TEXT];
    char statusText[96];

    // Private Methods
    int VisibleCols() const;
    int VisibleRows() const;
    int MaxScroll() const;

    void MarkDirty();
    void PushBanner();
    void PushPrompt();
    void ClearScreen();
    void ScrollUp();
    void ScrollDown();

    void AppendStatus();
    void AppendCharSafe(char c, int& idx, int limit);
    void BuildViewText();
    void UpdateView();

    char TranslateScancode(uint8_t scancode, bool shiftPressed) const;
    void ExecuteCommand(const char* cmd);
    void HandleEnter();
    void HandleScancode(uint8_t scancode, bool shiftPressed);
    void HandleScrollAction(int32_t scrollAction);
};

}  // namespace

#endif  // PROG_H
