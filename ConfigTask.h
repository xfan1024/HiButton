#ifndef __discoverytask_h__
#define __discoverytask_h__

#include <functional>
#include "task.h"

class ConfigTask : public Task
{
public:
    void on_config_wifi(const std::function<void(const char *ssid, const char *key)> &cb);
    ConfigTask();
    ~ConfigTask();
};

#endif
