/**
 * @file        prog.cpp
 * @brief       Hashx86 File Explorer
 *
 * @date        22/02/2026
 * @version     1.0.0
 */

#include <Hx86/Hgui/Hgui.h>
#include <Hx86/Hsyscalls/syscalls.h>
#include <Hx86/Hx86.h>
#include <Hx86/debug.h>
#include <Hx86/utils/string.h>

#define MAX_ENTRIES 64

static char currentPath[256] = "/";
static ListViewItemData dirEntries[MAX_ENTRIES];
static int dirEntryCount = 0;

// Forward declarations
class ExplorerApp;
static ExplorerApp* g_app = nullptr;

class ExplorerApp {
private:
    Window* mainWindow;
    Label* pathLabel;
    Label* statusLabel;
    HListView* fileList;
    Button* btnUp;
    Button* btnRefresh;
    Button* btnOpen;

public:
    ExplorerApp();
    void refreshDirectory();
    void navigateUp();
    void openSelected();
    void onListClick();
    void handleEvent(uint32_t widgetID, uint32_t eventType);

private:
    int loadDirectory(const char* path);
    bool isExecutable(const char* name);
};

ExplorerApp::ExplorerApp() {
    // Window dimensions
    const int32_t winW = 1000;
    const int32_t winH = 700;
    const int32_t pad = 10;                     // Edge padding
    const int32_t titleBarH = 28;               // Window title bar height
    const int32_t contentW = winW - (pad * 2);  // Usable content width
    const int32_t toolbarY = titleBarH + pad;   // Toolbar row Y
    const int32_t toolbarH = 26;
    const int32_t listY = toolbarY + toolbarH + pad;  // ListView Y
    const int32_t statusH = 18;
    const int32_t statusY = winH - statusH - pad;  // Status bar at bottom
    const int32_t listH = statusY - listY - pad;   // ListView fills remaining space

    mainWindow = new Window(desktop, 120, 300, winW, winH);
    mainWindow->setWindowTitle("Explorer");

    // Path bar spanning content width, inside toolbar row
    pathLabel = new Label(mainWindow, pad + 260, toolbarY + 2, contentW - 260, 20, "/");
    pathLabel->setSize(SMALL);

    // Toolbar buttons - left side
    btnUp = new Button(mainWindow, pad, toolbarY, 70, toolbarH, "Up");
    btnRefresh = new Button(mainWindow, pad + 78, toolbarY, 90, toolbarH, "Refresh");
    btnOpen = new Button(mainWindow, pad + 176, toolbarY, 70, toolbarH, "Open");

    // File list view - fills the main content area
    fileList = new HListView(mainWindow, pad, listY, contentW, listH);
    fileList->SetHeader("Name");

    // Status bar at bottom
    statusLabel = new Label(mainWindow, pad, statusY, contentW, statusH, "Ready");
    statusLabel->setSize(TINY);

    // Add all children into the window
    mainWindow->AddChild(pathLabel);
    mainWindow->AddChild(btnUp);
    mainWindow->AddChild(btnRefresh);
    mainWindow->AddChild(btnOpen);
    mainWindow->AddChild(fileList);
    mainWindow->AddChild(statusLabel);

    // Set up button callbacks
    btnUp->OnClick(this, [](void* inst) { static_cast<ExplorerApp*>(inst)->navigateUp(); });
    btnRefresh->OnClick(this,
                        [](void* inst) { static_cast<ExplorerApp*>(inst)->refreshDirectory(); });
    btnOpen->OnClick(this, [](void* inst) { static_cast<ExplorerApp*>(inst)->openSelected(); });
    fileList->OnClick(this, [](void* inst) { static_cast<ExplorerApp*>(inst)->onListClick(); });

    mainWindow->show();

    // Initial directory load
    refreshDirectory();
}

int ExplorerApp::loadDirectory(const char* path) {
    dirEntryCount = 0;

    int32_t fd = syscall_open(path, 0);
    if (fd < 0) {
        printf("[Explorer] Failed to open: %s\n", path);
        return -1;
    }

    char buffer[1024];
    while (dirEntryCount < MAX_ENTRIES) {
        int32_t bytes = syscall_getdents(fd, (struct linux_dirent*)buffer, sizeof(buffer));
        if (bytes <= 0) break;

        uint32_t offset = 0;
        while (offset < (uint32_t)bytes && dirEntryCount < MAX_ENTRIES) {
            struct linux_dirent* ent = (struct linux_dirent*)(buffer + offset);
            if (ent->d_name[0] != '\0') {
                ListViewItemData& item = dirEntries[dirEntryCount];

                // Copy name
                int j = 0;
                while (ent->d_name[j] && j < 63) {
                    item.name[j] = ent->d_name[j];
                    j++;
                }
                item.name[j] = 0;

                // Stat the entry for size and type
                char fullpath[256];
                memset(fullpath, 0, sizeof(fullpath));
                strcpy(fullpath, path);
                if (fullpath[strlen(fullpath) - 1] != '/') strcat(fullpath, "/");
                strcat(fullpath, item.name);

                struct stat st;
                item.size = 0;
                item.type = 0;  // default: file

                if (syscall_stat(fullpath, &st) == 0) {
                    item.size = st.st_size;
                    if (st.st_mode & 0x4000) {
                        item.type = 1;  // directory
                    } else if (isExecutable(item.name)) {
                        item.type = 2;  // executable
                    }
                }

                dirEntryCount++;
            }
            offset += ent->d_reclen;
        }
    }

    syscall_close(fd);
    return dirEntryCount;
}

bool ExplorerApp::isExecutable(const char* name) {
    int len = strlen(name);
    if (len < 5) return false;
    // Check for .BIN extension (Only need to check for Capital letters)
    if ((name[len - 4] == '.') && (name[len - 3] == 'B') && (name[len - 2] == 'I') &&
        (name[len - 1] == 'N')) {
        return true;
    }
    return false;
}

void ExplorerApp::refreshDirectory() {
    statusLabel->setText("Loading...");
    pathLabel->setText(currentPath);

    int count = loadDirectory(currentPath);

    if (count >= 0) {
        fileList->SetItems(dirEntries, dirEntryCount);

        // Build status message
        char statusMsg[64];
        memset(statusMsg, 0, sizeof(statusMsg));
        strcpy(statusMsg, "");
        char numStr[16];
        itoa(numStr, 10, dirEntryCount);
        strcat(statusMsg, numStr);
        strcat(statusMsg, " items");
        statusLabel->setText(statusMsg);
    } else {
        fileList->Clear();
        statusLabel->setText("Failed to open directory");
    }
}

void ExplorerApp::navigateUp() {
    int len = strlen(currentPath);
    if (len <= 1) return;  // Already at root

    // Remove trailing slash if any
    if (currentPath[len - 1] == '/' && len > 1) {
        currentPath[len - 1] = 0;
        len--;
    }

    // Find last slash
    int lastSlash = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (currentPath[i] == '/') {
            lastSlash = i;
            break;
        }
    }

    if (lastSlash == 0) {
        currentPath[0] = '/';
        currentPath[1] = 0;
    } else {
        currentPath[lastSlash] = 0;
    }

    refreshDirectory();
}

void ExplorerApp::openSelected() {
    int sel = fileList->GetSelectedIndex();
    if (sel < 0 || sel >= dirEntryCount) {
        statusLabel->setText("No item selected");
        return;
    }

    ListViewItemData& item = dirEntries[sel];

    if (item.type == 1) {
        // Navigate into directory
        if (strlen(currentPath) > 1) strcat(currentPath, "/");
        strcat(currentPath, item.name);
        refreshDirectory();
    } else if (item.type == 2) {
        // Launch executable
        char fullpath[256];
        memset(fullpath, 0, sizeof(fullpath));
        strcpy(fullpath, currentPath);
        if (fullpath[strlen(fullpath) - 1] != '/') strcat(fullpath, "/");
        strcat(fullpath, item.name);

        statusLabel->setText("Launching...");
        int32_t pid = syscall_execve(fullpath, nullptr, nullptr);
        if (pid > 0) {
            statusLabel->setText("Launched!");
        } else {
            statusLabel->setText("Launch failed");
        }
    } else {
        // Regular file - just show info
        char info[64];
        memset(info, 0, sizeof(info));
        strcpy(info, item.name);
        strcat(info, " selected");
        statusLabel->setText(info);
    }
}

void ExplorerApp::onListClick() {
    int sel = fileList->GetSelectedIndex();
    if (sel >= 0 && sel < dirEntryCount) {
        char info[64];
        memset(info, 0, sizeof(info));
        strcpy(info, dirEntries[sel].name);
        statusLabel->setText(info);

        printf("[Explorer] Selected: %s\n", dirEntries[sel].name);
    }
}

void ExplorerApp::handleEvent(uint32_t widgetID, uint32_t eventType) {
    if (widgetID == btnUp->ID)
        navigateUp();
    else if (widgetID == btnRefresh->ID)
        refreshDirectory();
    else if (widgetID == btnOpen->ID)
        openSelected();
    else if (widgetID == fileList->ID)
        onListClick();
}

extern "C" void _start(void* arg) {
    init_sys(arg);
    init_graphics();
    printf("[Explorer] Starting...\n");

    g_app = new ExplorerApp();
}
