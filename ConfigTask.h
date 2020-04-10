#ifndef __discoverytask_h__
#define __discoverytask_h__

#include <functional>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "common.h"
#include "task.h"

namespace configtask_detail
{

static constexpr uint16_t config_udp_port = 7866;
typedef StaticJsonDocument<256> JsonDocumentType;

enum
{
    CONFERR_DONOT_REPLY = -10000,
    CONFERR_OK = 0,
    CONFERR_FAIL,
    CONFERR_INVALID,
    CONFERR_NO_COMMAND,
};

class ConfigImpl : public TaskImpl {
public:
    void on_config_wifi(const std::function<void(const char *ssid, const char *key)> &cb)
    {
        m_config_wifi_cb = cb;
    }

protected:
    void on_start() override
    {
        m_udp.begin(config_udp_port);
    }

    void on_loop() override
    {
        char buffer[256];
        JsonDocumentType json;
        if (m_udp.parsePacket() == 0)
            return;

        size_t total = m_udp.readBytes((uint8_t*)buffer, sizeof(buffer) - 1);
        buffer[total] = 0;
        logv() << "received config packet(len=" << total << ") from " << m_udp.remoteIP() << ": " << buffer;
        if (deserializeJson(json, buffer))
        {
            logd() << "parse json fail";
            return;
        }

        int id = 0;
        bool id_exist = true;

        if (!json["method"].is<String>())
            return;

        if (json["id"].is<int>())
            id = json["id"];
        else
            id_exist = false;

        String str = json["method"].as<String>();
        int code = CONFERR_NO_COMMAND;

        if (str == "discovery")
            code = do_discovery(json);
        else if (str == "config_wifi")
            code = do_config_wifi(json);

        if (code != CONFERR_DONOT_REPLY)
        {
            if (code != CONFERR_OK)
                json.clear();
            if (id_exist)
                json["id"] = id;
            else
                json["id"] = nullptr;
            json["result"] = code;
            serializeJson(json, str);
            logv() << "reply: " << str;
            m_udp.beginPacket(m_udp.remoteIP(), m_udp.remotePort());
            m_udp.write(str.c_str(), str.length());
            m_udp.endPacket();
        }
    }

    void on_stop() override
    {
        m_udp.stop();
    }

    const char* get_name() const override
    {
        return "Config";
    }
private:
    int do_discovery(JsonDocumentType& json)
    {
        if (json["device_type"].is<String>() && json["device_type"].as<String>() != get_device_type())
            return CONFERR_DONOT_REPLY;

        if (json["device_id"].is<String>() && json["device_id"].as<String>() != get_device_id())
            return CONFERR_DONOT_REPLY;

        json.clear();
        json["device_type"] = get_device_type();
        json["device_id"] = get_device_id();
        return CONFERR_OK;
    }

    int do_config_wifi(JsonDocumentType& json)
    {
        String ssid, key;
        bool key_exist = false;
        if (!json["ssid"].is<String>())
        {
            logw() << "require ssid";
            return CONFERR_INVALID;
        }

        if (json["key"].is<String>())
            key_exist = true;

        ssid = json["ssid"].as<String>();
        if (key_exist)
        {
            key = json["key"].as<String>();
            if (key.length() < 8)
            {
                logw() << "key is too short";
                return CONFERR_INVALID;
            }
        }
        if (m_config_wifi_cb)
            m_config_wifi_cb(ssid.c_str(), key_exist ? key.c_str() : nullptr);
        json.clear();
        return CONFERR_OK;
    }

    WiFiUDP m_udp;
    std::function<void(const char *name, const char *key)> m_config_wifi_cb;
};

}

class ConfigTask : public Task
{
public:
    void on_config_wifi(const std::function<void(const char *ssid, const char *key)> &cb)
    {
        m_impl.on_config_wifi(cb);
    }

    ConfigTask() : Task(&m_impl)
    {}
private:
    configtask_detail::ConfigImpl m_impl;
};

#endif
