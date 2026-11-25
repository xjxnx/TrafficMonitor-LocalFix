#pragma once
#include <cstdint>

// Used only by the sampling worker. A transient failure does not reopen all devices.
class HardwareMonitorRecovery
{
public:
    bool Failed()
    {
        const bool first = m_failures == 0;
        if (m_failures < 3)
            ++m_failures;
        return first;
    }
    void Succeeded() { m_failures = 0; }
    bool TakeRetry(std::uint64_t now)
    {
        if (m_failures < 3 || now < m_next_retry)
            return false;
        m_next_retry = now + 60000;
        return true;
    }
private:
    unsigned m_failures{};
    std::uint64_t m_next_retry{};
};
