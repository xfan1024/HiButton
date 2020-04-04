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

void common_init()
{
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
