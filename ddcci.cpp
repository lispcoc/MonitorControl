#include <windows.h>
#include <string>
#include <vector>
# include<iostream>
# include<fstream>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include "ddcci.h"

#pragma comment(lib, "Dxva2.lib")

struct MonitorInfo {
    HMONITOR hMonitor;
    RECT rcMonitor;
    std::wstring deviceName;
};

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
    DWORD nMonitors;
    nMonitors = GetSystemMetrics(SM_CMONITORS);

    auto a = GetActiveMonitors();
    for( auto monitor : a) {
        PHYSICAL_MONITOR physMons[8];
        DWORD physMonCount = 0;
        if (GetNumberOfPhysicalMonitorsFromHMONITOR(monitor.hMonitor, &physMonCount) && physMonCount > 0) {
            if (GetPhysicalMonitorsFromHMONITOR(monitor.hMonitor, physMonCount, physMons)) {
                if (id == monitor.deviceName) {
                    DWORD pdwCurrentValue;
                    GetVCPFeatureAndVCPFeatureReply(physMons[0].hPhysicalMonitor, 0x60, NULL, &pdwCurrentValue, NULL);
                    SetVCPFeature(physMons[0].hPhysicalMonitor, 0x60, code);
                    //std::wofstream outputfile("test.txt");
                    //outputfile << monitor.deviceName << "\n";
                    //outputfile << pdwCurrentValue << "\n";
                    //outputfile << std::to_wstring(code) << "\n";
                    //outputfile.close();
                }
                DestroyPhysicalMonitors(physMonCount, physMons);
            }
        }
	}
}

