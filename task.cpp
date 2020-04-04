#include <debug.h>
#include "task.h"
#include "common.h"

void Task::start()
{
    common_panic_assert(m_actived == false);
    m_impl->on_start();
    m_actived = true;
}

void Task::stop()
{
    common_panic_assert(m_actived == true);
    m_impl->on_stop();
    m_actived = false;
}

void Task::loop()
{
    if (m_actived)
        m_impl->on_loop();
}

const char* Task::get_name() const
{
    return m_impl->get_name();
}

LogPrint TaskImpl::get_logger(unsigned int log_level)
{
    char ptrbuf[24];
    static const char *chrtable = "VDIWE";
    if (log_level < m_log_level)
        return LogPrint();
    Print& pt = get_debug_stream();
    sprintf(ptrbuf, "%p", reinterpret_cast<void*>(this));
    const char *ptr_print = ptrbuf;
    if (ptr_print[0] == '0' && ptr_print[1] == 'x')
        ptr_print += 2;
    pt << "[" << get_name() << "@" << ptr_print << "/" << chrtable[log_level] << "] ";
    return LogPrint(&pt, "\n", log_level >= LOG_WARNING);
}

void Task::set_log_level(unsigned int log_level)
{
    m_impl->m_log_level = log_level;
}

LogPrint TaskImpl::logv()
{
    return get_logger(LOG_VERBOSE);
}
LogPrint TaskImpl::logd()
{
    return get_logger(LOG_DEBUG);
}
LogPrint TaskImpl::logi()
{
    return get_logger(LOG_INFO);
}
LogPrint TaskImpl::logw()
{
    return get_logger(LOG_WARNING);
}
LogPrint TaskImpl::loge()
{
    return get_logger(LOG_ERROR);
}
