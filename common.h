#ifndef __common_h__
#define __common_h__

#include <Arduino.h>
#include "cppstream.h"

#define common_panic_message(msg) common_panic_func(msg, __FUNCTION__, __FILE__, __LINE__);
#define common_panic_assert(cond) do { \
    if (!(cond)) \
        common_panic_func("assert fail: " #cond, __FUNCTION__, __FILE__, __LINE__); \
} while (0)

void common_init();
void common_panic_func(const char *msg, const char *func, const char *file, int line);
Print& get_debug_stream();
Print& get_null_stream();

const char* get_device_type();
const char* get_device_id();
const char* get_softap_name();


#endif
