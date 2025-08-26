#include <windows.h>
#include <string>
#include <vector>
# include<iostream>
# include<fstream>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include "ddcci.h"

// Dxva2.lib�������N����K�v����
#pragma comment(lib, "Dxva2.lib")

// ���j�^�[���\����
struct MonitorInfo {
    HMONITOR hMonitor;
    RECT rcMonitor;
    std::wstring deviceName;
};

// �R�[���o�b�N�֐�
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

// �A�N�e�B�u�ȃ��j�^�[�ꗗ�擾�֐�
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
    // ��F�P�x��50�ɐݒ�iVCP�R�[�h 0x10�j
    DWORD nMonitors;
    nMonitors = GetSystemMetrics(SM_CMONITORS);

#if 0
    // ���ׂẴ��j�^�ɑ��M�i�K�v�ɉ����ď�������j
    for (DWORD i = 0; i < nMonitors; ++i) {
        PHYSICAL_MONITOR physMons[8];
        DWORD physMonCount = 0;
        HMONITOR hMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
        if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &physMonCount) && physMonCount > 0) {
            if (GetPhysicalMonitorsFromHMONITOR(hMonitor, physMonCount, physMons)) {
                SetVCPFeature(physMons[0].hPhysicalMonitor, 0x10, 50); // �P�x��50��
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

