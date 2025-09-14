#pragma once
#include <string>
#include <vector>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

typedef struct _MonitorInfo {
    HMONITOR hMonitor;
    RECT rcMonitor;
    std::wstring deviceName;
} MonitorInfo;

void SendDDCCommand(const WCHAR* name, const DWORD code);
std::vector<MonitorInfo> GetActiveMonitors();
