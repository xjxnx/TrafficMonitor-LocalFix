#include <Windows.h>
#include <cmath>
#include <cstdio>
#include "OpenHardwareMonitor/OpenHardwareMonitorApi.h"

int main()
{
    using namespace OpenHardwareMonitorApi;
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        auto monitor = CreateInstance();
        if (!monitor) { std::fwprintf(stderr, L"Create failed: %s\n", GetErrorMessage().c_str()); return 1; }
        monitor->SetCpuEnable(true);
        monitor->SetGpuEnable(true);
        monitor->SetHddEnable(false);
        monitor->SetMainboardEnable(false);
        for (int i = 0; i < 5; ++i)
        {
            auto start = GetTickCount64();
            monitor->GetHardwareInfo();
            auto error = GetErrorMessage();
            if (!error.empty()) { std::fwprintf(stderr, L"Sample failed: %s\n", error.c_str()); return 1; }
            float cpu = monitor->CpuTemperature(), gpu = monitor->GpuTemperature(), clock = monitor->CpuFreq();
            if (!std::isfinite(cpu) || !std::isfinite(gpu) || !std::isfinite(clock)) return 2;
            std::printf("cycle=%d sample=%d cpu=%.1f gpu=%.1f clock=%.3f duration=%llums\n", cycle, i, cpu, gpu, clock, GetTickCount64() - start);
            Sleep(200);
        }
        monitor->SetCpuEnable(false);
        monitor->SetGpuEnable(false);
    }
    std::puts("PASS: real CPU/GPU sampling and repeated enable/disable/recreate");
}
