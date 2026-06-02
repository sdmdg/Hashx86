/**
 * @file        prog.cpp
 * @brief       Terminal CLI [BIN]
 *
 * @date        06/03/2026
 * @version     1.1.0
 */

#include "include/prog.h"

namespace {

void TerminalBuffer::Init() {
    lineCount = 1;
    for (int i = 0; i < MAX_LINES; i++) {
        lengths[i] = 0;
        lines[i][0] = '\0';
    }
}

void TerminalBuffer::Clear() {
    Init();
}

int TerminalBuffer::CurrentIndex() const {
    return lineCount - 1;
}

void TerminalBuffer::NewLine() {
    if (lineCount < MAX_LINES) {
        lineCount++;
        int idx = lineCount - 1;
        lengths[idx] = 0;
        lines[idx][0] = '\0';
        return;
    }

    for (int i = 1; i < MAX_LINES; i++) {
        lengths[i - 1] = lengths[i];
        for (int j = 0; j <= MAX_COLS; j++) {
            lines[i - 1][j] = lines[i][j];
            if (lines[i][j] == '\0') break;
        }
    }

    lengths[MAX_LINES - 1] = 0;
    lines[MAX_LINES - 1][0] = '\0';
}

void TerminalBuffer::AppendChar(char c, int wrapCols) {
    int idx = CurrentIndex();
    if (lengths[idx] >= min_i(MAX_COLS, wrapCols)) {
        NewLine();
        idx = CurrentIndex();
    }

    if (lengths[idx] < MAX_COLS) {
        lines[idx][lengths[idx]++] = c;
        lines[idx][lengths[idx]] = '\0';
    }
}

void TerminalBuffer::AppendText(const char* text, int wrapCols) {
    for (int i = 0; text[i] != '\0'; i++) {
        AppendChar(text[i], wrapCols);
    }
}

void TerminalBuffer::Backspace(int protectColumns) {
    int idx = CurrentIndex();
    if (lengths[idx] > protectColumns) {
        lengths[idx]--;
        lines[idx][lengths[idx]] = '\0';
    }
}

TerminalApp::TerminalApp()
    : mainWindow(nullptr),
      terminalView(nullptr),
      statusLabel(nullptr),
      terminal(nullptr),
      scrollOffset(0),
      capsLock(false),
      inputStartColumn(0),
      viewDirty(true) {
    renderText[0] = '\0';
    statusText[0] = '\0';
}

TerminalApp::~TerminalApp() {
    if (mainWindow) delete mainWindow;
    if (terminal) delete terminal;
}

bool TerminalApp::Init() {
    terminal = new TerminalBuffer;
    if (!terminal) return false;
    terminal->Init();

    mainWindow = new Window(desktop, 120, 110, WIN_W, WIN_H);
    if (!mainWindow) return false;
    mainWindow->setWindowTitle("Terminal");

    terminalView = new TerminalView(mainWindow, TERM_X, TERM_Y, TERM_W, TERM_H, "");
    statusLabel = new Label(mainWindow, STATUS_X, STATUS_Y, STATUS_W, STATUS_H, "");

    if (!terminalView || !statusLabel) return false;

    terminalView->setSize(TINY);
    statusLabel->setSize(TINY);

    terminalView->OnKeyPress(this, [](void* instance, uint8_t scancode, bool shiftPressed) {
        ((TerminalApp*)instance)->HandleScancode(scancode, shiftPressed);
    });
    terminalView->OnClick(this, [](void* instance) {
        int32_t action = ((TerminalApp*)instance)->terminalView->consumeScrollAction();
        ((TerminalApp*)instance)->HandleScrollAction(action);
    });

    mainWindow->AddChild(terminalView);
    mainWindow->AddChild(statusLabel);

    mainWindow->show();

    PushBanner();
    PushPrompt();
    UpdateView();
    return true;
}

void TerminalApp::Run() {
    return;
}

int TerminalApp::VisibleCols() const {
    return max_i(20, (TERM_W - 20) / CHAR_W);
}

int TerminalApp::VisibleRows() const {
    return max_i(8, (TERM_H - 6) / CHAR_H);
}

int TerminalApp::MaxScroll() const {
    return max_i(0, terminal->lineCount - VisibleRows());
}

void TerminalApp::MarkDirty() {
    viewDirty = true;
}

void TerminalApp::PushBanner() {
    terminal->AppendText("Hashx86 Terminal", VisibleCols());
    terminal->NewLine();
    terminal->AppendText("Type to echo. Commands: clear, help", VisibleCols());
    terminal->NewLine();
}

void TerminalApp::PushPrompt() {
    if (terminal->lengths[terminal->CurrentIndex()] != 0) {
        terminal->NewLine();
    }
    terminal->AppendText("> ", VisibleCols());
    inputStartColumn = terminal->lengths[terminal->CurrentIndex()];
}

void TerminalApp::ClearScreen() {
    terminal->Clear();
    scrollOffset = 0;
    PushPrompt();
    MarkDirty();
}

void TerminalApp::ScrollUp() {
    scrollOffset = min_i(MaxScroll(), scrollOffset + 2);
    MarkDirty();
}

void TerminalApp::ScrollDown() {
    scrollOffset = max_i(0, scrollOffset - 2);
    MarkDirty();
}

void TerminalApp::AppendStatus() {
    statusText[0] = '\0';

    strcat(statusText, "Lines: ");
    char num[16];
    itoa(num, 10, terminal->lineCount);
    strcat(statusText, num);

    strcat(statusText, "  Scroll: ");
    itoa(num, 10, scrollOffset);
    strcat(statusText, num);

    strcat(statusText, "  F11/F12 or scrollbar");
}

void TerminalApp::AppendCharSafe(char c, int& idx, int limit) {
    if (idx < limit - 1) {
        renderText[idx++] = c;
        renderText[idx] = '\0';
    }
}

void TerminalApp::BuildViewText() {
    renderText[0] = '\0';
    int idx = 0;

    int rows = VisibleRows();
    int cols = VisibleCols();
    int firstLine = max_i(0, terminal->lineCount - rows - scrollOffset);

    for (int r = 0; r < rows; r++) {
        int lineIdx = firstLine + r;
        if (lineIdx < terminal->lineCount) {
            int len = min_i(cols, terminal->lengths[lineIdx]);
            for (int c = 0; c < len; c++) {
                AppendCharSafe(terminal->lines[lineIdx][c], idx, MAX_RENDER_TEXT);
            }
        }

        if (r != rows - 1) {
            AppendCharSafe('\n', idx, MAX_RENDER_TEXT);
        }
    }
}

void TerminalApp::UpdateView() {
    if (scrollOffset > MaxScroll()) {
        scrollOffset = MaxScroll();
    }

    BuildViewText();
    AppendStatus();

    terminalView->setText(renderText);
    terminalView->setScrollMeta(terminal->lineCount, VisibleRows(), scrollOffset);
    statusLabel->setText(statusText);

    viewDirty = false;
}

char TerminalApp::TranslateScancode(uint8_t scancode, bool shiftPressed) const {
    if (scancode >= 128) return 0;

    char base = 0;
    if (is_alpha(kNormalMap[scancode])) {
        base = kNormalMap[scancode];
        bool uppercase = (shiftPressed && !capsLock) || (!shiftPressed && capsLock);
        if (uppercase) {
            base = to_upper(base);
        }
    } else {
        base = shiftPressed ? kShiftMap[scancode] : kNormalMap[scancode];
    }

    return base;
}

void TerminalApp::HandleScancode(uint8_t sc, bool shiftPressed) {
    if (sc == SC_ESC) {
        syscall_exit(0);
    } else if (sc == SC_CAPSLOCK) {
        capsLock = !capsLock;
    } else if (sc == SC_BACKSPACE) {
        terminal->Backspace(inputStartColumn);
        scrollOffset = 0;
    } else if (sc == SC_ENTER) {
        HandleEnter();
    } else if (sc == SC_F11) {
        ScrollUp();
    } else if (sc == SC_F12) {
        ScrollDown();
    } else {
        char c = TranslateScancode(sc, shiftPressed);
        if (c >= 32 && c <= 126) {
            terminal->AppendChar(c, VisibleCols());
            scrollOffset = 0;
        }
    }

    if (scrollOffset > MaxScroll()) {
        scrollOffset = MaxScroll();
    }

    MarkDirty();
    UpdateView();
}

void TerminalApp::HandleScrollAction(int32_t scrollAction) {
    if (scrollAction == 0) return;

    if (scrollAction <= -1000000) {
        // Absolute offset from drag: encoded as -(1000000 + offset)
        int absOffset = -(scrollAction + 1000000);
        if (absOffset < 0) absOffset = 0;
        if (absOffset > MaxScroll()) absOffset = MaxScroll();
        scrollOffset = absOffset;
        MarkDirty();
    } else if (scrollAction > 0) {
        if (scrollAction >= 5) {
            // Page scroll
            scrollOffset = min_i(MaxScroll(), scrollOffset + VisibleRows());
        } else {
            ScrollUp();
        }
    } else if (scrollAction < 0) {
        if (scrollAction <= -5) {
            scrollOffset = max_i(0, scrollOffset - VisibleRows());
        } else {
            ScrollDown();
        }
        MarkDirty();
    }

    if (viewDirty) {
        UpdateView();
    }
}

void TerminalApp::ExecuteCommand(const char* cmd) {
    if (string_eq(cmd, "clear")) {
        terminal->Clear();
        PushPrompt();
        scrollOffset = 0;
        return;
    }

    if (string_eq(cmd, "help")) {
        terminal->AppendText("Commands: clear, help", VisibleCols());
        terminal->NewLine();
        PushPrompt();
        scrollOffset = 0;
        return;
    }

    if (cmd[0] != '\0') {
        terminal->AppendText("echo: ", VisibleCols());
        terminal->AppendText(cmd, VisibleCols());
        terminal->NewLine();
    }

    PushPrompt();
    scrollOffset = 0;
}

void TerminalApp::HandleEnter() {
    int idx = terminal->CurrentIndex();
    char cmd[MAX_COLS + 1];
    int out = 0;

    for (int i = inputStartColumn; i < terminal->lengths[idx] && out < MAX_COLS; i++) {
        cmd[out++] = terminal->lines[idx][i];
    }
    cmd[out] = '\0';

    terminal->NewLine();
    ExecuteCommand(cmd);
}

}  // namespace

extern "C" void _start(void* arg) {
    init_sys(arg);
    init_graphics();

    TerminalApp* app = new TerminalApp();
    if (!app) {
        syscall_exit(1);
    }

    if (!app->Init()) {
        syscall_exit(1);
    }

    app->Run();
}
