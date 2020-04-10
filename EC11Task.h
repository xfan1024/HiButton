#ifndef __ec11task_h__
#define __ec11task_h__

#include <functional>
#include "ButtonTask.h"
#include "edgedetector.h"

namespace ec11task_detail
{

enum
{
    S_free,
    S_rotate_left,
    S_rotate_right,
    S_wait_free,
};

class EC11Impl : public TaskImpl {
public:
    EC11Impl(int pin_sw, int pin_a, int pin_b) : m_button_task(pin_sw, LOW, INPUT_PULLUP)
    {
        m_pin_a = pin_a;
        m_pin_b = pin_b;
    }

    void on_rotate_left(const ButtonTask::click_callback_type& cb)
    {
        m_rotate_left_cb = cb;
    }

    void on_rotate_right(const ButtonTask::click_callback_type& cb)
    {
        m_rotate_right_cb = cb;
    }

    void on_click(const ButtonTask::click_callback_type& cb)
    {
        m_button_task.on_click_callback(cb);
    }

    void on_double_click(const ButtonTask::click_callback_type& cb)
    {
        m_button_task.on_double_click_callback(cb);
    }

    void on_long_press(const ButtonTask::click_callback_type& cb)
    {
        m_button_task.on_long_press_callback(cb);
    }

private:
    int m_pin_a;
    int m_pin_b;
    typedef EdgeDetectorTmpl<3> EdgeDetector;
    EdgeDetector m_detector_a;
    EdgeDetector m_detector_b;
    int m_state;
    ButtonTask::click_callback_type m_rotate_left_cb;
    ButtonTask::click_callback_type m_rotate_right_cb;
    ButtonTask m_button_task;

protected:
    void on_start() override
    {
        m_button_task.start();
        pinMode(m_pin_a, INPUT_PULLUP);
        pinMode(m_pin_b, INPUT_PULLUP);

        m_detector_a.start(LOW);
        m_detector_b.start(LOW);
        m_state = S_wait_free;
    }

    void on_loop() override
    {
        m_button_task.loop();
        int a, b;
        a = m_detector_a.input_value(digitalRead(m_pin_a));
        b = m_detector_b.input_value(digitalRead(m_pin_b));

        if (a == HIGH && b == HIGH)
        {
            m_state = S_free;
            return;
        }

        switch (m_state)
        {
        case S_free:
            if (a == LOW && b == HIGH)
            {
                m_state = S_rotate_left;
                break;
            }
            if (a == HIGH && b == LOW)
            {
                m_state = S_rotate_right;
                break;
            }
            m_state = S_wait_free;
            break;
        case S_rotate_left:
        case S_rotate_right:
            if (a != b)
                break;
            if (m_state == S_rotate_left && m_rotate_left_cb)
                m_rotate_left_cb();
            if (m_state == S_rotate_right && m_rotate_right_cb)
                m_rotate_right_cb();
            m_state = S_wait_free;
            break;
        default:
            break;
        }
    }

    void on_stop() override
    {
        m_button_task.stop();
    }

    const char* get_name() const override
    {
        return "EC11";
    }
};

}

class EC11Task : public Task
{
public:
    EC11Task(int pin_sw, int pin_a, int pin_b) : m_impl(pin_sw, pin_a, pin_b), Task(&m_impl)
    {}

    void on_rotate_left(const ButtonTask::click_callback_type& cb)
    {
        m_impl.on_rotate_left(cb);
    }

    void on_rotate_right(const ButtonTask::click_callback_type& cb)
    {
        m_impl.on_rotate_right(cb);
    }

    void on_click(const ButtonTask::click_callback_type& cb)
    {
        m_impl.on_click(cb);
    }

    void on_double_click(const ButtonTask::click_callback_type& cb)
    {
        m_impl.on_double_click(cb);
    }

    void on_long_press(const ButtonTask::click_callback_type& cb)
    {
        m_impl.on_long_press(cb);
    }

private:
    ec11task_detail::EC11Impl m_impl;
};

#endif
