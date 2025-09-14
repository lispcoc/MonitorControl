#include <windows.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "ddcci.h"

#pragma comment(lib, "Dxva2.lib")

POINTL GetDisplayPositionByName(const WCHAR* DisplayName) {
    UINT32 pathCount = 0, modeCount = 0;
    LONG ret;

    GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount);

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    ret = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), NULL);
    if (ret != ERROR_SUCCESS) {
        return POINTL({-1, -1});
    }

    // ディスプレイ名を検索、比較
    for (UINT32 index = 0; index < pathCount; index++) {
        DISPLAYCONFIG_TARGET_DEVICE_NAME dn;
        dn.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        dn.header.adapterId = paths[index].targetInfo.adapterId;
        dn.header.id = paths[index].targetInfo.id;
        dn.header.size = sizeof(dn);
        DisplayConfigGetDeviceInfo((DISPLAYCONFIG_DEVICE_INFO_HEADER*)&dn);
        if (wcscmp(DisplayName, dn.monitorFriendlyDeviceName) == 0) {
            auto p = modes[paths[index].sourceInfo.modeInfoIdx].sourceMode.position;
            return modes[paths[index].sourceInfo.modeInfoIdx].sourceMode.position;
        }
    }

    return POINTL({ -1, -1 });
}

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

void SendDDCCommand(const WCHAR* name, const DWORD code) {
	auto rect = GetDisplayPositionByName(name);
    auto a = GetActiveMonitors();
	std::sort(a.begin(), a.end(), [](const MonitorInfo& m1, const MonitorInfo& m2) {
        if (m1.rcMonitor.left != m2.rcMonitor.left) {
            return m1.rcMonitor.left < m2.rcMonitor.left;
        }
        return m1.rcMonitor.top < m2.rcMonitor.top;
	});
    for( auto monitor : a) {
		if (monitor.rcMonitor.left == rect.x && monitor.rcMonitor.top == rect.y) {
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
	}
}

