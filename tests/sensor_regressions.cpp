#include <cstdio>
#include <cstdlib>
#include "../OpenHardwareMonitorApi/OpenHardwareMonitorImp.h"
using namespace TrafficMonitorTests;

static void Check(bool value, const char* message)
{
    if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
namespace OpenHardwareMonitorApi
{
    class MonitorSensorTests
    {
    public:
        static void Run()
        {
            COpenHardwareMonitor monitor;
            float value = -1;
            Check(!monitor.TryGetSensorValue(nullptr, value), "null sensor");
            Check(!monitor.TryGetSensorValue(gcnew FakeSensor(SensorType::Temperature, L"CPU", Nullable<float>()), value), "missing reading");
            Check(!monitor.TryGetSensorValue(gcnew FakeSensor(SensorType::Temperature, L"CPU", Single::NaN), value), "NaN reading");
            Check(!monitor.TryGetSensorValue(gcnew FakeSensor(SensorType::Temperature, L"CPU", Single::PositiveInfinity), value), "infinite reading");
            Check(monitor.TryGetSensorValue(gcnew FakeSensor(SensorType::Load, L"CPU", 0.0f), value) && value == 0, "valid zero load");
            auto empty = gcnew FakeHardware(gcnew array<ISensor^>(0));
            Check(!monitor.GetCPUFreq(empty, value) && value == -1, "no clock sensors");
            auto clock = gcnew FakeHardware(gcnew array<ISensor^> {
                gcnew FakeSensor(SensorType::Clock, L"Bus Speed", 100.0f),
                gcnew FakeSensor(SensorType::Clock, L"Core #1", 3200.0f),
                gcnew FakeSensor(SensorType::Clock, L"Core #2", Nullable<float>()) });
            Check(monitor.GetCPUFreq(clock, value) && value > 3.19f && value < 3.21f, "average valid core clocks only");
            Check(!monitor.GetCPUFreq(empty, value) && value == -1, "old clocks do not survive a missing sample");
            auto load = gcnew FakeHardware(gcnew array<ISensor^> {
                gcnew FakeSensor(SensorType::Load, L"CPU Core #1", 5.0f),
                gcnew FakeSensor(SensorType::Load, L"CPU Total", 75.0f) });
            Check(monitor.GetCpuUsage(load, value) && value == 75.0f, "CPU total instead of first core load");
            auto temperatures = gcnew FakeHardware(gcnew array<ISensor^> {
                gcnew FakeSensor(SensorType::Temperature, L"Core #1", Nullable<float>()),
                gcnew FakeSensor(SensorType::Temperature, L"Core #2", 52.0f) });
            Check(monitor.GetCpuTemperature(temperatures, value) && value == 52.0f, "missing temperatures are not averaged as zero");
            Check(!monitor.GetGpuUsage(empty, value) && value == -1, "unavailable GPU load is not zero load");
            std::map<std::wstring, float> disks;
            monitor.InsertValueToMap(disks, L"Identical disk", 40);
            monitor.InsertValueToMap(disks, L"Identical disk", 41);
            monitor.InsertValueToMap(disks, L"Identical disk", 42);
            Check(disks.size() == 3 && disks[L"Identical disk #2"] == 42, "three identical disk names retain all readings");
        }
    };
}
int main()
{
    OpenHardwareMonitorApi::MonitorSensorTests::Run();
    for each (String^ name in LibreHardwareMonitor::Hardware::Computer::typeid->Assembly->GetManifestResourceNames())
        Check(name->IndexOf(L"WinRing", StringComparison::OrdinalIgnoreCase) < 0 && !name->EndsWith(L".sys"), "no embedded WinRing0 or driver image");
    std::puts("PASS: null/NaN/infinite sensors, valid zero, missing/stale clocks, CPU total, temperature averaging, duplicate disks, no embedded WinRing0");
}
