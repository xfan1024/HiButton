#ifndef __task_h__
#define __task_h__

#include <array>
#include <Arduino.h>

enum
{
    LOG_VERBOSE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
};

class LogPrint : public Print
{
public:
    LogPrint(Print *real, const char *end, bool flush = false) : m_real(real), m_end(end), m_flush(flush)
    {}

    LogPrint() : LogPrint(nullptr, nullptr)
    {}

    ~LogPrint()
    {
        if (!m_real)
            return;
        m_real->write(m_end);
        if (m_flush)
            m_real->flush();
    }
    virtual size_t write(uint8_t dat) override
    {
        if (!m_real)
            return 1;
        return m_real->write(dat);
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        if (!m_real)
            return size;
        return m_real->write(buffer, size);
    }

    virtual void flush() override
    {
        if (!m_real)
            return;
        m_real->flush();
    }

private:
    Print *m_real;
    const char *m_end;
    bool m_flush;
};

class TaskImpl
{
protected:
    virtual void on_start() = 0;
    virtual void on_stop() = 0;
    virtual void on_loop() = 0;
    virtual const char *get_name() const;
    LogPrint logv();
    LogPrint logd();
    LogPrint logi();
    LogPrint logw();
    LogPrint loge();
private:
    friend class Task;
    LogPrint get_logger(unsigned int log_level);
    unsigned int m_log_level {LOG_INFO};
};

class Task
{
public:
    void start();
    void stop();
    void loop();
    const char *get_name() const;
    bool is_start();
    void set_log_level(unsigned int log_level);
protected:
    Task(TaskImpl *impl) : m_impl(impl)
    {}
    TaskImpl *m_impl;
private:
    bool m_actived { false };
};

#endif
