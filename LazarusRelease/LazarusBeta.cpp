#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#pragma comment(lib,"d3d11.lib")
#include<windows.h>
#include<string>
#include<vector>
#include <tlhelp32.h>
#include<fstream>
#include <cwchar>
#include <WtsApi32.h>
#include "resource.h"
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
//Lazarus function
void SetProcessState(DWORD targetPID, bool freeze);
DWORD GetPIDByName(const wchar_t* processName);
void rad(std::vector<std::wstring> list, bool freeze);
void rad(std::vector<std::wstring> list, bool freeze, DWORD GameT);
std::wstring GetNameByPID(DWORD PID);
DWORD GetPIDByHWND(HWND win);
//Window function
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(int, char**)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(101)), nullptr, nullptr, nullptr, L"LazarusWindow", nullptr};
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Lazarus", WS_OVERLAPPEDWINDOW, 100, 100, (int)(750 * main_scale), (int)(630 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);      
    style.FontScaleDpi = main_scale;       
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool done = false;
    std::vector<std::wstring> BlackList;
    std::vector<std::wstring> GameList;
    static char buf[256] = "";
    static char but[256] = "";
    RegisterHotKey(NULL, 8, MOD_NOREPEAT, VK_F8);
    RegisterHotKey(NULL, 10, MOD_NOREPEAT, VK_F10);
    MSG msg = { 0 };

    HWND activeWin = nullptr;
    DWORD activeWinPID = NULL;
    bool isbgpaused = false;
    bool isgameactive = false;
    DWORD lastProtectedPID = GetPIDByHWND(GetForegroundWindow());
    bool ispress = false;
    bool ispressf = false;
    UINT_PTR timeID = 0;
    std::wifstream fileBL("BlackList.txt");
    std::wstring line;
    if (fileBL.is_open()) {
        while (std::getline(fileBL, line)) {
            while (!line.empty() && (line.back() == L'\r' || line.back() == L' ')) {
                line.pop_back();
            }
            if (!line.empty() && line[0] == 0xFEFF) {
                line.erase(0, 1);
            }

            if (line.length() > 0) {
                BlackList.push_back(line);
            }
        }
    }
    fileBL.close();
    std::wifstream fileTG("Trigger.txt");
    if (fileTG.is_open()) {
        while (std::getline(fileTG, line)) {
            while (!line.empty() && (line.back() == L'\r' || line.back() == L' ')) {
                line.pop_back();
            }
            if (!line.empty() && line[0] == 0xFEFF) {
                line.erase(0, 1);
            }

            if (line.length() > 0) {
                GameList.push_back(line);
            }
        }
    }
    fileTG.close();
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_HOTKEY) {
                if (msg.wParam == 8) {
                    ispress = true;
                    ispressf = false;
                    timeID = SetTimer(NULL, 0, 350, NULL);
                }
                if (msg.wParam == 10)
                {
                    ispress = false;
                    ispressf = true;
                    isbgpaused = false;
                    KillTimer(NULL, timeID);
                    rad(BlackList, false);

                }
            }
            else if (msg.message == WM_TIMER) {
                if (ispress == true && ispressf == false) {
                    activeWin = GetForegroundWindow();
                    activeWinPID = GetPIDByHWND(activeWin);
                    for (int i = 0; i < GameList.size(); i++) {
                        if (_wcsicmp(GetNameByPID(activeWinPID).c_str(), GameList[i].c_str()) == 0) {
                            isgameactive = true;
                            break;
                        }
                    }
                    if (isgameactive == true && (isbgpaused == false || lastProtectedPID != activeWinPID)) {
                        isbgpaused = true;
                        lastProtectedPID = GetPIDByHWND(GetForegroundWindow());
                        rad(BlackList, isbgpaused, activeWinPID);
                    }
                    else if (isgameactive == false && isbgpaused == true) {
                        isbgpaused = false;
                        rad(BlackList, isbgpaused);
                    }
                    isgameactive = false;
                }
            }


            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Lazarus", nullptr, flags);
        ImGui::Text("Please end the program before the close ");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::BeginDisabled(ispress == true);
        if (ImGui::Button("Start/F8", ImVec2(200, 50))) {
            ispress = true;
            ispressf = false;
            timeID = SetTimer(NULL, 0, 350, NULL);
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(ispress == false);
        if (ImGui::Button("Stop/F10", ImVec2(200, 50))) {
            ispress = false;
            ispressf = true;
            isbgpaused = false;
            KillTimer(NULL, timeID);
            rad(BlackList, false);
        }
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::EndDisabled();
        ImGui::SetNextItemWidth(408);
        ImGui::InputTextWithHint("##BadProcess","Enter here name of app'discord.exe'/Number of app in list", buf, sizeof(buf));
        if (ImGui::Button("Add bad process")) {
            if (strlen(buf) > 0) {
                std::string inputSTR(buf);
                std::wstring wInputSTR(inputSTR.begin(), inputSTR.end());
                BlackList.push_back(wInputSTR);
                std::wofstream fileBL("BlackList.txt");
                for (short i = 0; BlackList.size() > i; i++) {
                    fileBL << BlackList[i];
                    fileBL << std::endl;
                }
                fileBL.close();
                memset(buf, 0, sizeof(buf));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete bad process(number)"))
        {
            if (strlen(buf) > 0) {
                short index = std::atoi(buf);
                if (index > 0 && index <= BlackList.size()) {
                    BlackList.erase(BlackList.begin() + index - 1);
                    std::wofstream fileBL("BlackList.txt");
                    for (short i = 0; BlackList.size() > i; i++) {
                        fileBL << BlackList[i];
                        fileBL << std::endl;
                    }
                    fileBL.close();
                }
                memset(buf, 0, sizeof(buf));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear bad process list")) {
            ImGui::OpenPopup("ConfirmClearBadList");
        }
        ImGui::BeginChild("ScrollRegion2", ImVec2(105, 30), true);
        ImGui::Text("Trash apps:");
        ImGui::EndChild();
        ImGui::BeginChild("ScrollRegion", ImVec2(250, 150), true);
        for (int i = 0; BlackList.size() > i; i++)
        {
            std::string out(BlackList[i].begin(), BlackList[i].end());
            ImGui::Text("%d. %s", i + 1, out.c_str());
        }
        ImGui::EndChild();
        ImGui::Spacing();
        ImGui::SetNextItemWidth(408);
        ImGui::InputTextWithHint("##TriggerProcess", "Enter here name of app'discord.exe'/Number of app in list", but, sizeof(but));
        if (ImGui::Button("Add trigger process")) {
            if (strlen(but) > 0) {
                std::string inputSTR1(but);
                std::wstring wInputSTR1(inputSTR1.begin(), inputSTR1.end());
                GameList.push_back(wInputSTR1);
                std::wofstream fileTG("Trigger.txt");
                for (short i = 0; GameList.size() > i; i++) {
                    fileTG << GameList[i];
                    fileTG << std::endl;
                }
                fileTG.close();
                memset(but, 0, sizeof(but));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete trigger process(number)"))
        {
            if (strlen(but) > 0) {
                short index = std::atoi(but);
                if (index > 0 && index <= GameList.size()) {
                    GameList.erase(GameList.begin() + index - 1);
                    std::wofstream fileTG("Trigger.txt");
                    for (short i = 0; GameList.size() > i; i++) {
                        fileTG << GameList[i];
                        fileTG << std::endl;
                    }
                    fileTG.close();
                }
                memset(but, 0, sizeof(but));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear game process list")) {
            ImGui::OpenPopup("ConfirmClearGameList");
        }
        ImGui::BeginChild("ScrollRegionNew", ImVec2(105, 30), true);
        ImGui::Text("Trigger apps:");
        ImGui::EndChild();
        ImGui::BeginChild("ScrollRegion1", ImVec2(250, 150), true);
        for (int i = 0; GameList.size() > i; i++)
        {
            std::string out1(GameList[i].begin(), GameList[i].end());
            ImGui::Text("%d. %s", i + 1, out1.c_str());
        }
        ImGui::EndChild();
        if (ImGui::BeginPopupModal("ConfirmClearBadList", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {  
            ImGui::Text("Do you sure that want to clear bad process list?");
            if (ImGui::Button("Yes")) {
                BlackList.clear();
                std::wofstream fileBL("BlackList.txt");
                fileBL <<"";
                fileBL.close();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopupModal("ConfirmClearGameList", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Do you sure that want to clear game process list?");
            if (ImGui::Button("Yes")) {
                GameList.clear();
                std::wofstream fileTG("Trigger.txt");
                fileTG << "";
                fileTG.close();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::End();
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        HRESULT hr = g_pSwapChain->Present(1, 0);
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
void SetProcessState(DWORD targetPID, bool freeze) {
    HANDLE OM;
    if (freeze == true) {
        OM = OpenProcess(PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION, false, targetPID);
        if (OM != 0) {
            SetProcessWorkingSetSize(OM, (SIZE_T)-1, (SIZE_T)-1);
            CloseHandle(OM);
        }
    }
    HANDLE Snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    if (Thread32First(Snap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == targetPID) {
                HANDLE hThreat = OpenThread(THREAD_SUSPEND_RESUME, false, te32.th32ThreadID);
                if (hThreat != NULL) {
                    if (freeze == true) {
                        SuspendThread(hThreat);
                    }
                    else {
                        while ((int)ResumeThread(hThreat) > 1) {}

                    }CloseHandle(hThreat);
                }
            }
        } while (Thread32Next(Snap, &te32));
        { CloseHandle(Snap); }
    }
}
DWORD GetPIDByName(const wchar_t* processName) {
    HANDLE process1 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pr32;
    pr32.dwSize = sizeof(PROCESSENTRY32);
    DWORD nPID = 0;
    if (Process32First(process1, &pr32)) {
        do {
            if (_wcsicmp(pr32.szExeFile, processName) == 0) {
                nPID = pr32.th32ProcessID;
                break;
            }
        } while (Process32Next(process1, &pr32));
    }
    CloseHandle(process1);
    return nPID;
}
void rad(std::vector<std::wstring> list, bool freeze) {
    HANDLE all = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    PROCESSENTRY32 al32;
    al32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(all, &al32)) {
        do {
            for (int i = 0; i < list.size(); i++) {
                if (_wcsicmp(al32.szExeFile, list[i].c_str()) == 0) {
                    SetProcessState(al32.th32ProcessID, freeze);
                }
            }
        } while (Process32Next(all, &al32));

    }
    CloseHandle(all);
}
void rad(std::vector<std::wstring> list, bool freeze, DWORD GameT) {
    HANDLE all = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    PROCESSENTRY32 al32;
    al32.dwSize = sizeof(PROCESSENTRY32);
    std::wstring name = GetNameByPID(GameT);
    if (Process32First(all, &al32)) {
        do {
            for (int i = 0; i < list.size(); i++) {
                if (_wcsicmp(al32.szExeFile, list[i].c_str()) == 0) {
                    if (_wcsicmp(al32.szExeFile, name.c_str()) != 0) {
                        SetProcessState(al32.th32ProcessID, freeze);
                    }
                    else { SetProcessState(al32.th32ProcessID, false); }
                }
            }
        } while (Process32Next(all, &al32));

    }
    CloseHandle(all);
}
std::wstring GetNameByPID(DWORD PID) {
    HANDLE process1 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pr32;
    pr32.dwSize = sizeof(PROCESSENTRY32);
    std::wstring Name;
    if (Process32First(process1, &pr32)) {
        do {
            if (pr32.th32ProcessID == PID) {
                Name = pr32.szExeFile;
                break;
            }
        } while (Process32Next(process1, &pr32));
    }
    CloseHandle(process1);
    return Name;
}
DWORD GetPIDByHWND(HWND win) {
    DWORD winPID = 0;
    GetWindowThreadProcessId(win, &winPID);
    return winPID;

}