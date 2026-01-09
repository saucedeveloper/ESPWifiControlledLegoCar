#ifndef CONTROL_TIMING
#define CONTROL_TIMING

#include <cstdint>

struct ControlSetting
{
    uint16_t period;
    float average;
};

class ControlTiming
{
public:
    ControlTiming(ControlSetting setting);

    bool Tick(unsigned long now, float& outSignalValue);

    void ResetState();

    ControlSetting& Setting();
    const ControlSetting& Setting() const;

private:
    unsigned long m_LastTime;
    uint16_t m_CurrentIndex;
    ControlSetting m_Setting;
};

ControlTiming::ControlTiming(ControlSetting setting)
    : m_LastTime{UINT16_MAX}, m_CurrentIndex{0}, m_Setting{setting}
{
}

inline bool ControlTiming::Tick(unsigned long now, float& outSignalValue)
{
    long deltaTime = (long)now - (long)m_LastTime;
    if (deltaTime < -1 || 1 < deltaTime)
        Serial.printf("ControlTiming warning: last time to now = %ldms\n", deltaTime);

    if (now <= m_LastTime && m_LastTime != UINT16_MAX)
    {
        outSignalValue = 0.0F;
        return false;
    }

    float totalValue = m_Setting.period * m_Setting.average;
    float remainingValue = totalValue - static_cast<float>(m_CurrentIndex);

    if (remainingValue <= 0.0F)
        outSignalValue = 0.0F;
    else if (remainingValue <= 1.0F)
        outSignalValue = remainingValue;
    else
        outSignalValue = 1.0F;

    m_CurrentIndex = (m_CurrentIndex + 1) % m_Setting.period;
    m_LastTime = now;
    return true;
}

inline void ControlTiming::ResetState()
{
    m_LastTime = UINT16_MAX;
    m_CurrentIndex = 0;
}

inline ControlSetting& ControlTiming::Setting() { return m_Setting; }

inline const ControlSetting& ControlTiming::Setting() const { return m_Setting; }

#endif // CONTROL_TIMING
