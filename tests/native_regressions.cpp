#include <Windows.h>
#include <iphlpapi.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include "../TrafficMonitor/MonitorThreadSignal.h"
#include "../TrafficMonitor/NetworkTableReader.h"
#pragma comment(lib, "iphlpapi.lib")

static void Check(bool value, const char* message)
{
    if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

static int reads;
static DWORD WINAPI GrowingTable(PMIB_IFTABLE table, PULONG bytes, BOOL)
{
    ++reads;
    if (reads <= 2) { *bytes = sizeof(MIB_IFTABLE) * (reads + 1); return ERROR_INSUFFICIENT_BUFFER; }
    table->dwNumEntries = 1;
    table->table[0].dwIndex = 73;
    return NO_ERROR;
}
static DWORD WINAPI FailingTable(PMIB_IFTABLE, PULONG bytes, BOOL)
{
    if (++reads == 1) { *bytes = sizeof(MIB_IFTABLE) * 4; return ERROR_INSUFFICIENT_BUFFER; }
    return ERROR_ACCESS_DENIED;
}
static DWORD WINAPI EndlessGrowth(PMIB_IFTABLE, PULONG bytes, BOOL)
{
    ++reads;
    *bytes += sizeof(MIB_IFTABLE);
    return ERROR_INSUFFICIENT_BUFFER;
}
static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w, LPARAM l)
{
    if (message == WM_APP + 1) return 42;
    return DefWindowProcW(window, message, w, l);
}

int main()
{
    {
        CMonitorThreadSignal signal;
        Check(signal.IsValid(), "event creation");
        std::atomic<int> samples{0};
        std::thread worker([&] { while (signal.WaitForRequest()) ++samples; });
        Sleep(60);
        Check(samples == 0, "idle worker does not sample");
        signal.Request();
        for (int i = 0; i < 100 && samples == 0; ++i) Sleep(5);
        Check(samples == 1, "request wakes idle worker");
        signal.Stop();
        worker.join();
    }
    {
        CMonitorThreadSignal signal;
        signal.Request();
        signal.Stop();
        Check(!signal.WaitForRequest(), "stop wins over queued request");
    }
    {
        // Previously a 1-second exit timeout could destroy this worker's owner.
        CMonitorThreadSignal signal;
        HANDLE entered = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        std::atomic<bool> completed{false};
        std::thread worker([&] {
            while (signal.WaitForRequest()) {
                SetEvent(entered);
                Sleep(1150);
                completed = true;
            }
        });
        signal.Request();
        Check(WaitForSingleObject(entered, 3000) == WAIT_OBJECT_0, "slow sample begins");
        signal.Stop();
        Check(WaitForMonitorThread(worker.native_handle()), "join slow worker");
        Check(completed, "slow callback finishes before owner destruction");
        worker.join();
        CloseHandle(entered);
    }
    {
        WNDCLASSW cls{};
        cls.lpfnWndProc = WindowProc;
        cls.lpszClassName = L"TrafficMonitorRegressionWindow";
        Check(RegisterClassW(&cls) != 0, "register message window");
        HWND window = CreateWindowW(cls.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
        Check(window != nullptr, "create message window");
        std::atomic<LRESULT> reply{0};
        std::thread worker([&] { reply = SendMessageW(window, WM_APP + 1, 0, 0); });
        Check(WaitForMonitorThread(worker.native_handle()), "UI join dispatches worker SendMessage");
        worker.join();
        Check(reply == 42, "worker receives synchronous UI reply");
        DestroyWindow(window);
        UnregisterClassW(cls.lpszClassName, nullptr);
    }
    MIB_IFTABLE* table = nullptr;
    DWORD size = sizeof(MIB_IFTABLE);
    reads = 0;
    Check(RefreshNetworkTable(table, size, GrowingTable) == NO_ERROR, "retry adapter growth during read");
    Check(reads == 3 && table->table[0].dwIndex == 73, "publish complete grown table");
    MIB_IFTABLE* original = table;
    DWORD original_size = size;
    reads = 0;
    Check(RefreshNetworkTable(table, size, FailingTable) == ERROR_ACCESS_DENIED, "return read failure");
    Check(table == original && size == original_size && table->table[0].dwIndex == 73, "failed resize retains previous allocation");
    reads = 0;
    Check(RefreshNetworkTable(table, size, EndlessGrowth) == ERROR_RETRY, "bound retries for changing adapters");
    Check(reads == 5 && table == original, "bounded growth retains previous allocation");
    free(table);
    std::puts("PASS: worker wake/stop, slow shutdown, UI-message shutdown, network growth/failure/retry regressions");
}
