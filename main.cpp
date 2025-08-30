#include <windows.h>
#include <shellapi.h>
#include <fstream>
#include <codecvt>
#include "ddcci.h"
#include "json.h"
#include "json_fwd.h"
#include "resource.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_KEY       1000
#define ID_TRAY_EXIT      1001
#define ID_TRAY_SEND_DDC  1002

HINSTANCE hInst;
NOTIFYICONDATA nid;
HWND hWnd;

nlohmann::json config;

nlohmann::json ReadJsonFile(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        return nlohmann::json();
    }
    nlohmann::json j;
    ifs >> j;
    return j;
}

static std::wstring ConvertUTF8ToWstring(const std::string& src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(src);
}

void SendGlobalKey(WORD keyCode) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;

    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void ShowTrayMenu(HWND hwnd) {
    config = ReadJsonFile("config.json");
    std::string id;
	config.at("id").get_to(id);

    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
	size_t i = 0;
    std::vector<size_t> codes;
    for (auto p : config.at("ports")) {
        std::string name;
        size_t code;
        p.at("name").get_to(name);
        p.at("code").get_to(code);
        InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_SEND_DDC + i, ConvertUTF8ToWstring(name).c_str());
        codes.push_back(code);
        ++i;
	}
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_KEY, L"ScrLock");
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hwnd);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);

    if (cmd >= ID_TRAY_SEND_DDC && cmd < (ID_TRAY_SEND_DDC + codes.size())) {
        SendDDCCommand(ConvertUTF8ToWstring(id), codes[cmd - ID_TRAY_SEND_DDC]);
    }
    else if (cmd == ID_TRAY_KEY) {
        SendGlobalKey(VK_SCROLL);
        SendGlobalKey(VK_SCROLL);
    }
    else if (cmd == ID_TRAY_EXIT) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
    }
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAYICON) {
        if (lParam == WM_RBUTTONUP) {
            ShowTrayMenu(hwnd);
        }
    }
    if (msg == WM_DESTROY) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    hInst = hInstance;
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayDDCApp";
    RegisterClass(&wc);

    hWnd = CreateWindow(wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wcscpy_s(nid.szTip, L"DDC/CI Tray App");
    Shell_NotifyIcon(NIM_ADD, &nid);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
