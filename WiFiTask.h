#ifndef __wifitask_h__
#define __wifitask_h__

#include "task.h"

class WiFiTask : public Task {
public:
    void configure(const char *ssid, const char *key);
    void configure_reset();
    WiFiTask();
    ~WiFiTask();
};

#endif
