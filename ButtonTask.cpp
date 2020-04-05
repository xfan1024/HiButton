#include "ButtonTask.h"
#include "common.h"

#define DEBOUNCING_TIME 30
#define DOUBLE_CLICK_INTERVAL_MAX 300

enum
{
    S_unpressed_free,
    S_pressed_debouncing,
    S_pressed_confirmed,
    S_unpressed_wait_next,
    S_pressed_double_click,
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
        "pressed_double_click",
    };
    if (state >= S_max)
        return "unknown";
    return strtab[state];
}

class ButtonImpl : public TaskImpl
{
public:
    ButtonImpl(int pin, int pressed_level, int pin_mode)
    {
        m_pin = pin;
        m_pin_mode = pin_mode;
        m_pressed_level = pressed_level;
    }

    void on_click_callback(const ButtonTask::click_callback_type& cb)
    {
        m_click_cb = cb;
    }

    void on_double_click_callback(const ButtonTask::click_callback_type& cb)
    {
        m_double_click_cb = cb;
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
            if (elapsed < DEBOUNCING_TIME)
                return;
            change_state(pressed ? S_pressed_confirmed : S_unpressed_free);
            break;
        case S_pressed_confirmed:
            if (pressed)
                return;
            change_state(S_unpressed_wait_next);
            m_privous_time = current;
            break;
        case S_unpressed_wait_next:
            if (pressed && elapsed <= DOUBLE_CLICK_INTERVAL_MAX)
            {
                logv() << "double-click interval: " << elapsed << "ms";
                change_state(S_pressed_double_click);
                if (m_double_click_cb)
                {
                    logd() << "trigger double-click event";
                    m_double_click_cb();
                }
            }
            else if (!pressed && elapsed > DOUBLE_CLICK_INTERVAL_MAX)
            {
                change_state(S_unpressed_free);
                if (m_click_cb)
                {
                    logd() << "trigger click event";
                    m_click_cb();
                }
            }
            break;
        case S_pressed_double_click:
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
    ButtonTask::click_callback_type m_click_cb;
    ButtonTask::click_callback_type m_double_click_cb;
};


static inline ButtonImpl* as_button_impl(TaskImpl *impl)
{
    return static_cast<ButtonImpl*>(impl);
}

ButtonTask::ButtonTask(int pin, int pressed_level, int pin_mode) : Task(new ButtonImpl(pin, pressed_level, pin_mode))
{}

ButtonTask::~ButtonTask()
{
    delete as_button_impl(m_impl);
}

void ButtonTask::on_click_callback(const click_callback_type& cb)
{
    as_button_impl(m_impl)->on_click_callback(cb);
}

void ButtonTask::on_double_click_callback(const click_callback_type& cb)
{
    as_button_impl(m_impl)->on_double_click_callback(cb);
}

