#include <ESP8266WiFi.h>
#include "WiFiTask.h"
#include "common.h"
#include "config.h"

class WiFiImpl : public TaskImpl {
protected:
    void on_start() override
    {
        WiFi.enableAP(false);
        WiFi.begin(CONF_WIFI_SSID, CONF_WIFI_KEY);
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
        WiFi.enableSTA(false);
    }

    const char* get_name() const override
    {
        return "WiFi";
    }
private:
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