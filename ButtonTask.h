#ifndef __buttontask_h__
#define __buttontask_h__

#include "task.h"
#include <functional>

class ButtonTask : public Task
{
public:
    typedef std::function<void()> click_callback_type;
    ButtonTask(int pin, int pressed_level, int pin_mode = INPUT);
    ~ButtonTask();

    void on_click_callback(const click_callback_type& cb);
    void on_double_click_callback(const click_callback_type& cb);
    void on_long_press_callback(const click_callback_type &cb);
};

#endif
