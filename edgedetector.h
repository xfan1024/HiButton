#ifndef __edgedetector_h__
#define __edgedetector_h__

#include <Arduino.h>

template <long interval_min>
class EdgeDetectorTmpl
{
public:
    enum class edge
    {
        none = 0,
        rising,
        falling,
    };

    void start(int initial = HIGH)
    {
        m_privous = millis();
        m_privous_value = initial;
    }

    int input_value(int v)
    {
        unsigned long now = millis();
        long diff = (long)(now - m_privous);
        if (diff < interval_min)
            return m_privous_value;
        if (m_privous_value == v)
        {
            m_privous = now - interval_min;
            return v;
        }
        m_privous = now;
        m_privous_value = v;
        return v;
    }

    edge detect_edge(int v)
    {
        unsigned long now = millis();
        long diff = (long)(now - m_privous);
        if (diff < interval_min)
            return edge::none;
        if (m_privous_value == v)
        {
            m_privous = now - interval_min;
            return edge::none;
        }
        m_privous = now;
        m_privous_value = v;
        return v == HIGH ? edge::rising : edge::falling;
    }

    void stop()
    {
    }

private:
    unsigned long m_privous;
    int m_privous_value;
};

#endif