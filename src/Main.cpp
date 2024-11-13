#include "gui.h"

#ifdef _CONSOLE
// For Console Subsystem (Debug), use `main`
int main(int, char**) {
#else
// For Windows Subsystem (Release), use `WinMain`
#include <Windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#endif

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, GUI::WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"GTA IV External CheatMenu", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"GTAIV External Cheat Menu", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);

    if (!GUI::Initialize(hwnd)) {
        GUI::Shutdown();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        GUI::ProcessInput();
        GUI::Render();
    }

    GUI::Shutdown();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

#ifdef _CONSOLE
    return 0;  // Return from `main` for Console
#else
    return 0;  // Return from `WinMain` for Windows
#endif

}
