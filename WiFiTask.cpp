#include <ESP8266WiFi.h>
#include "WiFiTask.h"
#include "common.h"
#include "config.h"

class WiFiImpl : public TaskImpl {
public:
    void configure(const char *ssid, const char *key)
    {
        if(is_configured())
        {
            logw() << "WiFi is aready configured, ignore configure request";
            return;
        }
        if (key)
            WiFi.begin(ssid, key);
        else
            WiFi.begin(ssid);
        WiFi.setAutoConnect(true);
        logi() << "Connect to " << ssid;
        wifi_restart();
    }

    bool is_configured()
    {
#ifndef CONF_WIFI_SSID
        return WiFi.getAutoConnect();
#else
        return true;
#endif
    }

    void configure_reset()
    {
#ifndef CONF_WIFI_SSID
        WiFi.setAutoConnect(false);
        wifi_restart();
#endif
    }
protected:
    void on_start() override
    {
#ifdef CONF_WIFI_SSID
        WiFi.mode(WIFI_STA);
#ifdef CONF_WIFI_KEY
        WiFi.begin(CONF_WIFI_SSID, CONF_WIFI_KEY);
#else
        WiFi.begin(CONF_WIFI_SSID);
#endif
        logi() << "Connect to " << CONF_WIFI_SSID;
#else
        wifi_restart();
#endif
        mConnected = false;
    }

    void on_loop() override
    {
        bool connected = WiFi.isConnected();
        if (mConnected != connected)
        {
            mConnected = connected;
            if (connected)
            {
                logi() << "WiFi connected";
                logi() << "  SSID: " << WiFi.SSID();
                logi() << "    IP: " << WiFi.localIP();
                logi() << "    GW: " << WiFi.gatewayIP();
                logi() << "  MASK: " << WiFi.subnetMask();
            }
            else
            {
                logi() << "WiFi disconnected";
            }
        }
    }

    void on_stop() override
    {
        WiFi.mode(WIFI_OFF);
    }

    const char* get_name() const override
    {
        return "WiFi";
    }
private:
    void wifi_restart()
    {
        if (is_configured())
        {
            logi() << "WiFi start as Station";
            WiFi.mode(WIFI_STA);
        }
        else
        {
            logi() << "WiFi start as Access Point";
            WiFi.mode(WIFI_AP);
        }
    }
    bool mConnected;
};

static inline WiFiImpl* as_wifi_impl(TaskImpl *impl)
{
    return static_cast<WiFiImpl*>(impl);
}

WiFiTask::WiFiTask() : Task(new WiFiImpl)
{}

WiFiTask::~WiFiTask()
{
    delete as_wifi_impl(m_impl);
}

void WiFiTask::configure(const char *ssid, const char *key)
{
    as_wifi_impl(m_impl)->configure(ssid, key);
}

void WiFiTask::configure_reset()
{
    as_wifi_impl(m_impl)->configure_reset();
}