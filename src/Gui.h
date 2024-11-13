// gui.h
#pragma once

#include <d3d9.h>
#include "Memory.h"

class Memory;

namespace GUI {
    extern LPDIRECT3D9 g_pD3D;
    extern LPDIRECT3DDEVICE9 g_pd3dDevice;
    extern bool g_DeviceLost;
    extern UINT g_ResizeWidth, g_ResizeHeight;
    extern D3DPRESENT_PARAMETERS g_d3dpp;

    extern Memory memory;

    bool Initialize(HWND hwnd);
    void Shutdown();
    void Render();
    void ResetDevice();
    void ProcessInput();
    bool CreateDeviceD3D(HWND hwnd);
    void CleanupDeviceD3D();
    LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}
