// gui.cpp
#include "gui.h"
#include <windows.h>
#include <tchar.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GUI {
    LPDIRECT3D9 g_pD3D = nullptr;
    LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;
    bool g_DeviceLost = false;
    UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
    D3DPRESENT_PARAMETERS g_d3dpp = {};

    Memory memory;

    bool Initialize(HWND hwnd) {
        if (!CreateDeviceD3D(hwnd))
            return false;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX9_Init(g_pd3dDevice);
        return true;
    }

    void Shutdown() {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupDeviceD3D();
    }

    void Render() {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            static float Health, currentHealth = 0.0f;
            static float Armor, currentArmor = 0.0f;
            static int Money, currentMoney = 0;
            static int WantedLevel, currentWantedLevel = 0;

            static bool initialized = false;

            static bool GodMode = false;

            if (!initialized) {
                memory.ReadMemory(memory.Money->pointsAddress, Money);
                memory.ReadMemory(memory.Armor->pointsAddress, Armor);
                memory.ReadMemory(memory.Health->pointsAddress, Health);
                memory.ReadMemory(memory.WantedLevel->pointsAddress, WantedLevel);

                initialized = true;
            }
            else {
                memory.ReadMemory(memory.Money->pointsAddress, currentMoney);
                memory.ReadMemory(memory.Armor->pointsAddress, currentArmor);
                memory.ReadMemory(memory.Health->pointsAddress, currentHealth);
                memory.ReadMemory(memory.WantedLevel->pointsAddress, currentWantedLevel);
            }

            ImVec2 windowSize = ImGui::GetIO().DisplaySize;
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(windowSize);
            ImGui::Begin("##CheatMenu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

            // Money
            ImGui::Text("Money:");
            ImGui::SameLine();
            if (ImGui::InputInt("##Change Money", &Money, 1000, 100000))
                memory.WriteMemory(memory.Money->pointsAddress, Money);

            // Health
            ImGui::Text("Health:");
            ImGui::SameLine();
            if (ImGui::InputFloat("##Change Health", &Health, 1, 100))
                memory.WriteMemory(memory.Health->pointsAddress, Health);

            // Armor
            ImGui::Text("Armor:");
            ImGui::SameLine();
            if (ImGui::InputFloat("##Change Armor", &Armor, 1, 100))
                memory.WriteMemory(memory.Armor->pointsAddress, Armor);

            // Wanted Level
            ImGui::Text("Wanted:");
            ImGui::SameLine();
            if (ImGui::SliderInt("##Change Wanted", &WantedLevel, 0, 5))
                memory.WriteMemory(memory.WantedLevel->pointsAddress, WantedLevel);
            ImGui::SameLine();
            if (ImGui::Button("Max"))
                memory.WriteMemory(memory.WantedLevel->pointsAddress, 5);
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
                memory.WriteMemory(memory.WantedLevel->pointsAddress, 0);

            // God Mode
            ImGui::Checkbox("GodMode", &GodMode);
            if (GodMode)
            {
                memory.Freeze(memory.Health->pointsAddress, 200.0f);
                memory.Freeze(memory.Armor->pointsAddress, 100.0f);
            }

            // Update Money to reflect in-game value when user is not interacting with input
            if (!ImGui::IsItemActive()) {
                Money = currentMoney;
                Armor = currentArmor;
                Health = currentHealth;
                WantedLevel = currentWantedLevel;
            }

            ImGui::End();
        }

        ImGui::Render();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(114, 144, 154, 255);
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        if (g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr) == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    void ResetDevice() {
        ImGui_ImplDX9_InvalidateDeviceObjects();
        HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
        if (hr == D3DERR_INVALIDCALL)
            IM_ASSERT(0);
        ImGui_ImplDX9_CreateDeviceObjects();
    }

    void ProcessInput() {
        if (g_DeviceLost) {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) {
                ::Sleep(10);
                return;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }
    }

    bool CreateDeviceD3D(HWND hwnd) {
        if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
            return false;

        ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
        g_d3dpp.Windowed = TRUE;
        g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
        g_d3dpp.EnableAutoDepthStencil = TRUE;
        g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
        g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
            return false;

        return true;
    }

    void CleanupDeviceD3D() {
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
        if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
    }

    LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
                return 0;
            g_ResizeWidth = (UINT)LOWORD(lParam);
            g_ResizeHeight = (UINT)HIWORD(lParam);
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        case WM_GETMINMAXINFO: {
            MINMAXINFO* minMaxInfo = (MINMAXINFO*)lParam;
            minMaxInfo->ptMinTrackSize.x = 800; // Minimum width
            minMaxInfo->ptMinTrackSize.y = 600; // Minimum height
        } break;
        }
        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
