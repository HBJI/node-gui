#include "clipboard_win.h"
#include <stdio.h>

namespace clip {

static WinClipboardMonitor *self = NULL;
    
WinClipboardMonitor::WinClipboardMonitor(sigc::slot<void> slot) :
    slot_(slot)
{
    self = this;

    // Create window class
    WNDCLASS wcx = { 0 }; 

    wcx.lpfnWndProc = MainWndProc;     // points to window procedure
    wcx.lpszClassName = "WinClipboardMonitor";  // name of window class 
   
    // Register the window class. 
    RegisterClass(&wcx);

    handle_ = CreateWindow( 
        "WinClipboardMonitor", // name of window class 
        "WinClipboardMonitor", // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        CW_USEDEFAULT,       // default width 
        CW_USEDEFAULT,       // default height 
        (HWND) NULL,         // no owner window 
        (HMENU) NULL,        // use class menu 
        0,                   // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 

    if (!handle_) {
        fprintf(stderr, "Failed to create window");
    }
}


WinClipboardMonitor::~WinClipboardMonitor(void)
{
    DestroyWindow(handle_);
}

LRESULT CALLBACK WinClipboardMonitor::MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndNextViewer; 
    
    switch (uMsg) {
        case WM_CREATE: 
            hwndNextViewer = SetClipboardViewer(hwnd); 
            break;
        case WM_DESTROY:
            ChangeClipboardChain(hwnd, hwndNextViewer);
            break;
        case WM_CHANGECBCHAIN:
            if ((HWND) wParam == hwndNextViewer) 
                hwndNextViewer = (HWND) lParam; 
            else if (hwndNextViewer != NULL) 
                SendMessage(hwndNextViewer, uMsg, wParam, lParam); 
 
            break;
        case WM_DRAWCLIPBOARD:  // clipboard contents changed. 
            SendMessage(hwndNextViewer, uMsg, wParam, lParam);
            self->slot_();
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    }

    return (LRESULT) NULL;
}
}