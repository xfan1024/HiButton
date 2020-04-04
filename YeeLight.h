#ifndef __yeelight_h__
#define __yeelight_h__

#include <IPAddress.h>
#include "task.h"

class YeeLightTask : public Task
{
public:
    YeeLightTask();
    ~YeeLightTask();
    void set_remote(const IPAddress& remote);
    void bright_adjust(int val);
    void ct_adjust(int val);
    void power_toggle();
    void mode_toggle();
};

#endif
