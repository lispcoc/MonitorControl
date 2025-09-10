#include <string>
#include <vector>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

struct MonitorInfo {
    HMONITOR hMonitor;
    RECT rcMonitor;
    std::wstring deviceName;
};

#pragma once
void SendDDCCommand(ULONG id, const size_t code);
std::vector<MonitorInfo> GetActiveMonitors();
