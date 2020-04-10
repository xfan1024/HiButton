#ifndef __buttontask_h__
#define __buttontask_h__

#include <functional>
#include "common.h"
#include "task.h"

namespace buttontask_detail
{

static constexpr long debouncing_time = 30;
static constexpr long double_click_interval_max = 300;
static constexpr long long_press_min = 3000;

enum
{
    S_unpressed_free,
    S_pressed_debouncing,
    S_pressed_confirmed,
    S_unpressed_wait_next,
    S_pressed_wait_unpressed,
    S_max
};

static const char *str_state(unsigned int state)
{
    static const char *strtab[S_max] =
    {
        "unpressed_free",
        "pressed_debouncing",
        "pressed_confirmed",
        "unpressed_wait_next",
        "pressed_wait_unpressed",
    };
    if (state >= S_max)
        return "unknown";
    return strtab[state];
}

class ButtonImpl : public TaskImpl
{
public:
    typedef std::function<void()> click_callback_type;
    ButtonImpl(int pin, int pressed_level, int pin_mode)
    {
        m_pin = pin;
        m_pin_mode = pin_mode;
        m_pressed_level = pressed_level;
    }

    void on_click_callback(const click_callback_type& cb)
    {
        m_click_cb = cb;
    }

    void on_double_click_callback(const click_callback_type& cb)
    {
        m_double_click_cb = cb;
    }

    void on_long_press_callback(const click_callback_type& cb)
    {
        m_long_press_cb = cb;
    }

protected:
    void on_start() override
    {
        m_state = S_unpressed_free;
        pinMode(m_pin, m_pin_mode);
    }

    void on_loop() override
    {
        unsigned long current = millis();
        unsigned long elapsed = current - m_privous_time;
        bool pressed = is_pressed();
        switch (m_state)
        {
        case S_unpressed_free:
            if (!pressed)
                return;
            change_state(S_pressed_debouncing);
            m_privous_time = current;
            break;
        case S_pressed_debouncing:
            if (elapsed < debouncing_time)
                return;
            change_state(pressed ? S_pressed_confirmed : S_unpressed_free);
            break;
        case S_pressed_confirmed:
            if (pressed)
            {
                if (elapsed > long_press_min)
                {
                    if (m_long_press_cb)
                        m_long_press_cb();
                    change_state(S_pressed_wait_unpressed);
                }
                return;
            }
            change_state(S_unpressed_wait_next);
            m_privous_time = current;
            break;
        case S_unpressed_wait_next:
            if (pressed && elapsed <= double_click_interval_max)
            {
                logv() << "double-click interval: " << elapsed << "ms";
                change_state(S_pressed_wait_unpressed);
                if (m_double_click_cb)
                {
                    logd() << "trigger double-click event";
                    m_double_click_cb();
                }
            }
            else if (!pressed && elapsed > double_click_interval_max)
            {
                change_state(S_unpressed_free);
                if (m_click_cb)
                {
                    logd() << "trigger click event";
                    m_click_cb();
                }
            }
            break;
        case S_pressed_wait_unpressed:
            if (!pressed)
                change_state(S_unpressed_free);
            break;

        }

    }

    void on_stop() override
    {
    }

    const char* get_name() const override
    {
        return "Button";
    }

private:
    bool is_pressed()
    {
        return m_pressed_level == digitalRead(m_pin);
    }

    void change_state(unsigned int state)
    {
        logv() << "state change from " << m_state << "(" << str_state(m_state) << ") to " << state << "(" << str_state(state) << ")";
        m_state = state;
    }

    int m_pin;
    int m_pin_mode;
    int m_pressed_level;
    unsigned long m_privous_time;
    unsigned int m_state;
    click_callback_type m_click_cb;
    click_callback_type m_double_click_cb;
    click_callback_type m_long_press_cb;
};

}

class ButtonTask : public Task
{
public:
    typedef buttontask_detail::ButtonImpl::click_callback_type click_callback_type;

    ButtonTask(int pin, int pressed_level, int pin_mode = INPUT) : m_impl(pin, pressed_level, pin_mode), Task(&m_impl)
    {}

    void on_click_callback(const click_callback_type& cb)
    {
        m_impl.on_click_callback(cb);
    }

    void on_double_click_callback(const click_callback_type& cb)
    {
        m_impl.on_double_click_callback(cb);
    }

    void on_long_press_callback(const click_callback_type& cb)
    {
        m_impl.on_long_press_callback(cb);
    }
private:
    buttontask_detail::ButtonImpl m_impl;
};

#endif
