#include <ESP8266WiFi.h>
#include "common.h"

class NullStream : public Stream
{
public:
    virtual int available() override
    {
        return 0;
    }
    virtual int read() override
    {
        return 0;
    }
    virtual int peek() override
    {
        return -1;
    }
    virtual size_t write(uint8_t) override
    {
        return 1;
    }
};

static NullStream null_stream;
static HardwareSerial debug_serial(0);

static String device_id;
static String softap_name;

void common_init()
{
    device_id = WiFi.macAddress();
    device_id.replace(":", "");
    softap_name = get_device_type();
    softap_name += "-";
    softap_name += device_id.substring(device_id.length() - 4);

    debug_serial.begin(115200);
    debug_serial << "\n\nstartup\n";
}

Print& get_debug_stream()
{
    return debug_serial;
}

Print& get_null_stream()
{
    return null_stream;
}

void common_panic_func(const char *msg, const char *func, const char *file, int line)
{
    get_debug_stream() << "system panic\n"
                       << "  postion: " << file << ":" << line << '\n'
                       << "      msg: " << msg << '\n'
                       << "     func: " << func << '\n'
                       << "system will reboot";
    get_debug_stream() << '\n';
    get_debug_stream().flush();
    delay(10);
    ESP.restart();
    while (1);
}

const char* get_device_type()
{
    return "HiButton";
}

const char* get_device_id()
{
    return device_id.c_str();
}

const char* get_softap_name()
{
    return softap_name.c_str();
}