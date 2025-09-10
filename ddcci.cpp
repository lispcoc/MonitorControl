#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "ddcci.h"

#pragma comment(lib, "Dxva2.lib")

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

void SendDDCCommand(ULONG id, const size_t code) {
    auto a = GetActiveMonitors();
	std::sort(a.begin(), a.end(), [](const MonitorInfo& m1, const MonitorInfo& m2) {
        if (m1.rcMonitor.left != m2.rcMonitor.left) {
            return m1.rcMonitor.left < m2.rcMonitor.left;
        }
        return m1.rcMonitor.top < m2.rcMonitor.top;
	});
    for( auto monitor : a) {
		if (id == 0) {
            PHYSICAL_MONITOR physMons[8];
            DWORD physMonCount = 0;
            if (GetNumberOfPhysicalMonitorsFromHMONITOR(monitor.hMonitor, &physMonCount) && physMonCount > 0) {
                if (GetPhysicalMonitorsFromHMONITOR(monitor.hMonitor, physMonCount, physMons)) {
                    DWORD pdwCurrentValue;
                    GetVCPFeatureAndVCPFeatureReply(physMons[0].hPhysicalMonitor, 0x60, NULL, &pdwCurrentValue, NULL);
                    SetVCPFeature(physMons[0].hPhysicalMonitor, 0x60, code);
                    DestroyPhysicalMonitors(physMonCount, physMons);
                }
            }
            break;
        }
        id--;
	}
}

