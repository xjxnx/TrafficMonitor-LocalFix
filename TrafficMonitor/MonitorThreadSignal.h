#pragma once
#include <Windows.h>

// A manual stop event takes priority over coalesced sampling requests.
// The owner must join its worker before destroying these handles.
class CMonitorThreadSignal
{
public:
    CMonitorThreadSignal()
    {
        m_events[0] = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        m_events[1] = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    }
    ~CMonitorThreadSignal()
    {
        for (HANDLE event : m_events)
            if (event != nullptr)
                CloseHandle(event);
    }
    CMonitorThreadSignal(const CMonitorThreadSignal&) = delete;
    CMonitorThreadSignal& operator=(const CMonitorThreadSignal&) = delete;
    bool IsValid() const { return m_events[0] && m_events[1]; }
    void Request() { SetEvent(m_events[1]); }
    void Stop() { SetEvent(m_events[0]); }
    bool WaitForRequest() const
    {
        return WaitForMultipleObjects(2, m_events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1;
    }
private:
    HANDLE m_events[2]{};
};

// A worker can be in a synchronous SendMessage to this UI thread at exit.
// Dispatch sent messages while joining, without running posted UI commands.
inline bool WaitForMonitorThread(HANDLE thread)
{
    for (;;)
    {
        const DWORD result = MsgWaitForMultipleObjects(1, &thread, FALSE, INFINITE, QS_SENDMESSAGE);
        if (result == WAIT_OBJECT_0)
            return true;
        if (result != WAIT_OBJECT_0 + 1)
            return false;
        MSG message{};
        PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
    }
}
