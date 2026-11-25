// 这是主 DLL 文件。

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "OpenHardwareMonitorImp.h"
#include <vector>
#include <vcclr.h>

namespace OpenHardwareMonitorApi
{
    static std::wstring error_message;

    //将CRL的String类型转换成C++的std::wstring类型
    static std::wstring ClrStringToStdWstring(System::String^ str)
    {
        if (str == nullptr)
        {
            return std::wstring();
        }
        else
        {
            const wchar_t* chars = (const wchar_t*)(Runtime::InteropServices::Marshal::StringToHGlobalUni(str)).ToPointer();
            std::wstring os = chars;
            Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr((void*)chars));
            return os;
        }
    }


    std::shared_ptr<IOpenHardwareMonitor> CreateInstance()
    {
        error_message.clear();
        std::shared_ptr<IOpenHardwareMonitor> pMonitor;
        try
        {
            MonitorGlobal::Instance()->Init();
            pMonitor = std::make_shared<COpenHardwareMonitor>();
        }
        catch (System::Exception^ e)
        {
            error_message = ClrStringToStdWstring(e->Message);
        }
        return pMonitor;
    }

    std::wstring GetErrorMessage()
    {
        return error_message;
    }

    float COpenHardwareMonitor::CpuTemperature()
    {
        return m_cpu_temperature;
    }

    float COpenHardwareMonitor::GpuTemperature()
    {
        if (m_gpu_nvidia_temperature >= 0)
            return m_gpu_nvidia_temperature;
        else if (m_gpu_ati_temperature >= 0)
            return m_gpu_ati_temperature;
        else
            return m_gpu_intel_temperature;
    }

    float COpenHardwareMonitor::HDDTemperature()
    {
        return m_hdd_temperature;
    }

    float COpenHardwareMonitor::MainboardTemperature()
    {
        return m_main_board_temperature;
    }

    float COpenHardwareMonitor::GpuUsage()
    {
        if (m_gpu_nvidia_usage >= 0)
            return m_gpu_nvidia_usage;
        else if (m_gpu_ati_usage >= 0)
            return m_gpu_ati_usage;
        else
            return m_gpu_intel_usage;
    }

    float COpenHardwareMonitor::CpuFreq()
    {
            return m_cpu_freq;
    }

    float COpenHardwareMonitor::CpuUsage()
    {
        return m_cpu_usage;
    }

    const std::map<std::wstring, float>& COpenHardwareMonitor::AllHDDTemperature()
    {
        return m_all_hdd_temperature;
    }

    const std::map<std::wstring, float>& COpenHardwareMonitor::AllCpuTemperature()
    {
        return m_all_cpu_temperature;
    }

    const std::map<std::wstring, float>& COpenHardwareMonitor::AllHDDUsage()
    {
        return m_all_hdd_usage;
    }

    void COpenHardwareMonitor::SetCpuEnable(bool enable)
    {
        try { MonitorGlobal::Instance()->computer->IsCpuEnabled = enable; }
        catch (System::Exception^ e) { error_message = ClrStringToStdWstring(e->Message); }
    }

    void COpenHardwareMonitor::SetGpuEnable(bool enable)
    {
        try { MonitorGlobal::Instance()->computer->IsGpuEnabled = enable; }
        catch (System::Exception^ e) { error_message = ClrStringToStdWstring(e->Message); }
    }

    void COpenHardwareMonitor::SetHddEnable(bool enable)
    {
        try { MonitorGlobal::Instance()->computer->IsStorageEnabled = enable; }
        catch (System::Exception^ e) { error_message = ClrStringToStdWstring(e->Message); }
    }

    void COpenHardwareMonitor::SetMainboardEnable(bool enable)
    {
        try { MonitorGlobal::Instance()->computer->IsMotherboardEnabled = enable; }
        catch (System::Exception^ e) { error_message = ClrStringToStdWstring(e->Message); }
    }

    bool COpenHardwareMonitor::TryGetSensorValue(ISensor^ sensor, float& value)
    {
        if (sensor == nullptr)
            return false;
        auto snapshot = sensor->Value;
        if (!snapshot.HasValue)
            return false;
        value = snapshot.Value;
        return !System::Single::IsNaN(value) && !System::Single::IsInfinity(value);
    }

    bool COpenHardwareMonitor::GetCPUFreq(IHardware^ hardware, float& freq) {
        m_all_cpu_clock.clear();
        for (int i = 0; i < hardware->Sensors->Length; i++)
        {
            if (hardware->Sensors[i]->SensorType == SensorType::Clock)
            {
                String^ name = hardware->Sensors[i]->Name;
                float value;
                if (name != L"Bus Speed" && TryGetSensorValue(hardware->Sensors[i], value) && value > 0)
                    m_all_cpu_clock[ClrStringToStdWstring(name)] = value;
            }
        }
        float sum{};
        for (auto i : m_all_cpu_clock)
            sum += i.second;
        if (m_all_cpu_clock.empty())
        {
            freq = -1;
            return false;
        }
        freq = sum / m_all_cpu_clock.size() / 1000.0;
        return true;
    }

    bool COpenHardwareMonitor::GetCpuUsage(IHardware^ hardware, float& cpu_usage)
    {
        for (int i = 0; i < hardware->Sensors->Length; i++)
        {
            if (hardware->Sensors[i]->SensorType == SensorType::Load)
            {
                String^ name = hardware->Sensors[i]->Name;
                if (name == L"CPU Total")
                {
                    if (TryGetSensorValue(hardware->Sensors[i], cpu_usage))
                        return true;
                }
            }
        }
        return false;
    }

    bool COpenHardwareMonitor::GetHardwareTemperature(IHardware^ hardware, float& temperature)
    {
        temperature = -1;
        std::vector<float> all_temperature;
        float core_temperature{ -1 };
        System::String^ temperature_name;
        switch (hardware->HardwareType)
        {
        case HardwareType::Cpu:
            temperature_name = L"Core Average";
            break;
        case HardwareType::GpuNvidia: case HardwareType::GpuAmd: case HardwareType::GpuIntel:
            temperature_name = L"GPU Core";
            break;
        default:
            break;
        }
        for (int i = 0; i < hardware->Sensors->Length; i++)
        {
            //找到温度传感器
            if (hardware->Sensors[i]->SensorType == SensorType::Temperature)
            {
                float cur_temperture;
                if (!TryGetSensorValue(hardware->Sensors[i], cur_temperture))
                    continue;
                all_temperature.push_back(cur_temperture);
                if (hardware->Sensors[i]->Name == temperature_name) //如果找到了名称为temperature_name的温度传感器，则将温度保存到core_temperature里
                    core_temperature = cur_temperture;
            }
        }
        if (core_temperature >= 0)
        {
            temperature = core_temperature;
            return true;
        }
        if (!all_temperature.empty())
        {
            //如果有多个温度传感器，则取平均值
            float sum{};
            for (auto i : all_temperature)
                sum += i;
            temperature = sum / all_temperature.size();
            return true;
        }
        //如果没有找到温度传感器，则在SubHardware中寻找
        for (int i = 0; i < hardware->SubHardware->Length; i++)
        {
            if (GetHardwareTemperature(hardware->SubHardware[i], temperature))
                return true;
        }
        return false;
    }

    bool COpenHardwareMonitor::GetCpuTemperature(IHardware^ hardware, float& temperature)
    {
        temperature = -1;
        m_all_cpu_temperature.clear();
        for (int i = 0; i < hardware->Sensors->Length; i++)
        {
            //找到温度传感器
            if (hardware->Sensors[i]->SensorType == SensorType::Temperature)
            {
                String^ name = hardware->Sensors[i]->Name;
                //保存每个CPU传感器的温度
                float value;
                if (TryGetSensorValue(hardware->Sensors[i], value))
                    m_all_cpu_temperature[ClrStringToStdWstring(name)] = value;
            }
        }
        //计算平均温度
        if (!m_all_cpu_temperature.empty())
        {
            float sum{};
            for (const auto& item : m_all_cpu_temperature)
                sum += item.second;
            temperature = sum / m_all_cpu_temperature.size();
        }
        return temperature > 0;
    }

    bool COpenHardwareMonitor::GetGpuUsage(IHardware^ hardware, float& gpu_usage)
    {
        float usage_max = -1;
        for (int i = 0; i < hardware->Sensors->Length; i++)
        {
            //找到负载
            if (hardware->Sensors[i]->SensorType == SensorType::Load)
            {
                float cur_gpu_usage;
                if (!TryGetSensorValue(hardware->Sensors[i], cur_gpu_usage))
                    continue;
                if (hardware->Sensors[i]->Name == L"GPU Core")
                {
                    gpu_usage = cur_gpu_usage;
                    return true;
                }

                //计算最大值
                if (cur_gpu_usage > usage_max)
                    usage_max = cur_gpu_usage;
            }
        }
        gpu_usage = usage_max;
        return usage_max >= 0;
    }

    bool COpenHardwareMonitor::GetHddUsage(IHardware^ hardware, float& hdd_usage)
    {
        for (int i = 0; i < hardware->Sensors->Length; i++)
        {
            //找到负载
            if (hardware->Sensors[i]->SensorType == SensorType::Load)
            {
                if (hardware->Sensors[i]->Name == L"Total Activity")
                {
                    if (TryGetSensorValue(hardware->Sensors[i], hdd_usage))
                        return true;
                }
            }
        }
        return false;
    }

    COpenHardwareMonitor::COpenHardwareMonitor()
    {
        ResetAllValues();
    }

    COpenHardwareMonitor::~COpenHardwareMonitor()
    {
        try { MonitorGlobal::Instance()->UnInit(); }
        catch (System::Exception^ e) { error_message = ClrStringToStdWstring(e->Message); }
    }

    void COpenHardwareMonitor::ResetAllValues()
    {
        error_message.clear();
        m_cpu_temperature = -1;
        m_gpu_nvidia_temperature = -1;
        m_gpu_ati_temperature = -1;
        m_gpu_intel_temperature = -1;
        m_hdd_temperature = -1;
        m_main_board_temperature = -1;
        m_gpu_nvidia_usage = -1;
        m_gpu_ati_usage = -1;
        m_gpu_intel_usage = -1;
        m_all_hdd_temperature.clear();
        m_all_hdd_usage.clear();
        m_all_cpu_temperature.clear();
        m_all_cpu_clock.clear();
        m_cpu_freq = -1;
        m_cpu_usage = -1;
    }

    void COpenHardwareMonitor::InsertValueToMap(std::map<std::wstring, float>& value_map, const std::wstring& key, float value)
    {
        std::wstring unique_key = key;
        unsigned int suffix = 1;
        while (value_map.find(unique_key) != value_map.end())
            unique_key = key + L" #" + std::to_wstring(suffix++);
        value_map[unique_key] = value;
    }

    // Keep native driver faults local to the device whose update invoked the driver.
    struct DeviceUpdate
    {
        gcroot<IHardware^> hardware;
        gcroot<UpdateVisitor^> visitor;
    };

    static void __cdecl VisitDevice(DeviceUpdate* update)
    {
        IHardware^ hardware = update->hardware;
        hardware->Accept(update->visitor);
    }

#pragma managed(push, off)
    static DWORD GuardDeviceUpdate(void (__cdecl *visit)(DeviceUpdate*), DeviceUpdate* update)
    {
        __try
        {
            visit(update);
            return ERROR_SUCCESS;
        }
        __except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION || GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR)
            ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            return GetExceptionCode();
        }
    }
#pragma managed(pop)

    void COpenHardwareMonitor::ReadDevice(IHardware^ hardware)
    {
        switch (hardware->HardwareType)
        {
        case HardwareType::Cpu:
            if (m_cpu_temperature < 0)
                GetCpuTemperature(hardware, m_cpu_temperature);
            if (m_cpu_freq < 0)
                GetCPUFreq(hardware, m_cpu_freq);
            if (m_cpu_usage < 0)
                GetCpuUsage(hardware, m_cpu_usage);
            break;
        case HardwareType::GpuNvidia:
            if (m_gpu_nvidia_temperature < 0)
                GetHardwareTemperature(hardware, m_gpu_nvidia_temperature);
            if (m_gpu_nvidia_usage < 0)
                GetGpuUsage(hardware, m_gpu_nvidia_usage);
            break;
        case HardwareType::GpuAmd:
            if (m_gpu_ati_temperature < 0)
                GetHardwareTemperature(hardware, m_gpu_ati_temperature);
            if (m_gpu_ati_usage < 0)
                GetGpuUsage(hardware, m_gpu_ati_usage);
            break;
        case HardwareType::GpuIntel:
            if (m_gpu_intel_temperature < 0)
                GetHardwareTemperature(hardware, m_gpu_intel_temperature);
            if (m_gpu_intel_usage < 0)
                GetGpuUsage(hardware, m_gpu_intel_usage);
            break;
        case HardwareType::Storage:
        {
            float cur_hdd_temperature = -1;
            GetHardwareTemperature(hardware, cur_hdd_temperature);
            //m_all_hdd_temperature[ClrStringToStdWstring(hardware->Name)] = cur_hdd_temperature;
            InsertValueToMap(m_all_hdd_temperature, ClrStringToStdWstring(hardware->Name), cur_hdd_temperature);
            float cur_hdd_usage = -1;
            GetHddUsage(hardware, cur_hdd_usage);
            //m_all_hdd_usage[ClrStringToStdWstring(hardware->Name)] = cur_hdd_usage;
            InsertValueToMap(m_all_hdd_usage, ClrStringToStdWstring(hardware->Name), cur_hdd_usage);
            if (m_hdd_temperature < 0)
                m_hdd_temperature = cur_hdd_temperature;
        }
        break;
        case HardwareType::Motherboard:
            if (m_main_board_temperature < 0)
                GetHardwareTemperature(hardware, m_main_board_temperature);
            break;
        default:
            break;
        }
    }

    void COpenHardwareMonitor::ReadDevices(System::Collections::Generic::IList<IHardware^>^ devices, UpdateVisitor^ visitor)
    {
        for each (IHardware^ hardware in devices)
        {
            try
            {
                DeviceUpdate update;
                update.hardware = hardware;
                update.visitor = visitor;
                DWORD code = GuardDeviceUpdate(&VisitDevice, &update);
                if (code == ERROR_SUCCESS)
                    ReadDevice(hardware);
                else if (error_message.empty())
                    error_message = ClrStringToStdWstring(String::Format(L"{0}: sensor update failed (0x{1:X8})", hardware->HardwareType, code));
            }
            catch (System::Exception^ e)
            {
                if (error_message.empty())
                    error_message = ClrStringToStdWstring(String::Format(L"{0}: {1}", hardware->HardwareType, e->Message));
            }
        }
    }

    void COpenHardwareMonitor::GetHardwareInfo()
    {
        ResetAllValues();
        try
        {
            auto monitor = MonitorGlobal::Instance();
            ReadDevices(monitor->computer->Hardware, monitor->updateVisitor);
        }
        catch (System::Exception^ e)
        {
            error_message = ClrStringToStdWstring(e->Message);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    MonitorGlobal::MonitorGlobal()
    {

    }

    MonitorGlobal::~MonitorGlobal()
    {

    }

    void MonitorGlobal::Init()
    {
        if (computer != nullptr)
            return;
        updateVisitor = gcnew UpdateVisitor();
        computer = gcnew Computer();
        try { computer->Open(); }
        catch (System::Exception^)
        {
            try { computer->Close(); } catch (System::Exception^) {}
            computer = nullptr;
            updateVisitor = nullptr;
            throw;
        }
    }

    void MonitorGlobal::UnInit()
    {
        if (computer == nullptr)
            return;
        try { computer->Close(); }
        finally
        {
            computer = nullptr;
            updateVisitor = nullptr;
        }
    }

}
