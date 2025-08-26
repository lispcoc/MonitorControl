#include <windows.h>
#include <string>
#include <vector>
# include<iostream>
# include<fstream>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include "ddcci.h"

// Dxva2.libをリンクする必要あり
#pragma comment(lib, "Dxva2.lib")

// モニター情報構造体
struct MonitorInfo {
    HMONITOR hMonitor;
    RECT rcMonitor;
    std::wstring deviceName;
};

// コールバック関数
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM dwData) {
    std::vector<MonitorInfo>* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);

    MONITORINFOEXW mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(hMonitor, &mi)) {
        MonitorInfo info;
        info.hMonitor = hMonitor;
        info.rcMonitor = mi.rcMonitor;
        info.deviceName = mi.szDevice;
        monitors->push_back(info);
    }
    return TRUE;
}

// アクティブなモニター一覧取得関数
std::vector<MonitorInfo> GetActiveMonitors() {
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
    return monitors;
}

std::wstring GetMonitorName(HMONITOR hMonitor) {
    MONITORINFOEXW mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(hMonitor, &mi)) {
        return std::wstring(mi.szDevice);
    }
    return L"";
}

void SendDDCCommand(const std::wstring &id, const size_t code) {
    // 例：輝度を50に設定（VCPコード 0x10）
    DWORD nMonitors;
    nMonitors = GetSystemMetrics(SM_CMONITORS);

#if 0
    // すべてのモニタに送信（必要に応じて条件分岐）
    for (DWORD i = 0; i < nMonitors; ++i) {
        PHYSICAL_MONITOR physMons[8];
        DWORD physMonCount = 0;
        HMONITOR hMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
        if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &physMonCount) && physMonCount > 0) {
            if (GetPhysicalMonitorsFromHMONITOR(hMonitor, physMonCount, physMons)) {
                SetVCPFeature(physMons[0].hPhysicalMonitor, 0x10, 50); // 輝度を50に
                DestroyPhysicalMonitors(physMonCount, physMons);
            }
        }
    }
#endif

    auto a = GetActiveMonitors();
    std::wofstream outputfile("test.txt");
    for( auto monitor : a) {
        PHYSICAL_MONITOR physMons[8];
        DWORD physMonCount = 0;
        if (GetNumberOfPhysicalMonitorsFromHMONITOR(monitor.hMonitor, &physMonCount) && physMonCount > 0) {
            if (GetPhysicalMonitorsFromHMONITOR(monitor.hMonitor, physMonCount, physMons)) {
                if (id == monitor.deviceName) {
                    DWORD pdwCurrentValue;
                    GetVCPFeatureAndVCPFeatureReply(physMons[0].hPhysicalMonitor, 0x60, NULL, &pdwCurrentValue, NULL);
                    outputfile << monitor.deviceName << "\n";
                    outputfile << pdwCurrentValue << "\n";
                    SetVCPFeature(physMons[0].hPhysicalMonitor, 0x60, code);
                }
                DestroyPhysicalMonitors(physMonCount, physMons);
            }
        }
	}
    outputfile.close();
}

