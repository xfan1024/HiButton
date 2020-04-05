#include <string>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include <memory>
#include "yeelight.h"
#include "common.h"

constexpr unsigned int heartbeat_interval = 5*1000;
constexpr unsigned int timeout_ms = 10*1000;

enum
{
    S_stop = 0,
    S_disconnect,
    S_connecting,
    S_ready,
    S_busy,
    S_disconnecting,
    S_max
};

const char *str_state(unsigned int state)
{
    static const char *strtab[S_max] =
    {
        "stop",
        "disconnect",
        "connecting",
        "ready",
        "busy",
        "disconnecting",
    };
    if (state >= S_max)
        return "unknown";
    return strtab[state];
}


typedef StaticJsonDocument<256> JsonDocumentType;

class YeeLightImpl : public TaskImpl
{
public:
    YeeLightImpl()
    {
        // m_client.reset(new AsyncClient);
        m_client.onConnect([this](void*, AsyncClient*){
            on_connect();
        }, nullptr);
        m_client.onDisconnect([this](void*, AsyncClient*){
            on_disconnect();
        }, nullptr);
        m_client.onError([this](void*, AsyncClient*, err_t error){
            logi() << "onError: " << error << '(' << m_client.errorToString(error) << ')';
            // on_disconnect();
        }, nullptr);
        m_client.onData([this](void*, AsyncClient*, void *data, size_t len){
            on_data(data, len);
        }, nullptr);
    }

    void set_remote(const IPAddress &remote)
    {
        m_remote = remote;
    }

    bool connected()
    {
        return m_state == S_ready || m_state == S_busy;
    }

    void power_toggle()
    {
        if (!connected())
            return;
        logi() << "power toggle";
        JsonDocumentType request;
        send_request("toggle", request, &YeeLightImpl::cb_donot_care);
    }

    void mode_toggle()
    {
        // set_power('on', 'smooth', 500, 5 or 1)
        if (!connected())
            return;
        m_night_mode = !m_night_mode;
        JsonDocumentType request;
        request.createNestedArray("params");
        request["params"][0] = "on";
        request["params"][1] = "smooth";
        request["params"][2] = 300;
        request["params"][3] = m_night_mode ? 5 : 1;
        logi() << "mode toggle: " << (m_night_mode ? "night" : "normal");
        send_request("set_power", request, &YeeLightImpl::cb_donot_care);
    }

protected:
    void on_start() override
    {
        change_state(S_disconnect);
    }

    void on_loop() override
    {
        switch (m_state)
        {
            case S_disconnect:
            {
                if (m_remote == IPAddress())
                    break;
                if (m_client.connect(m_remote, 55443)) {
                    logd() << "async connect to " << m_remote.toString();
                    change_state(S_connecting);
                }
                break;
            }
            case S_ready:
            {
                if (millis() - m_last_active > heartbeat_interval) {
                    send_heartbeat();
                }
            }
            case S_busy:
            {
                if (millis() - m_last_active > timeout_ms) {
                    close_connection();
                }
            }

        }
    }

    void on_stop() override
    {
        change_state(S_stop);
        close_connection();
    }

    const char* get_name() const override
    {
        return "YeeLight";
    }
private:
    typedef void (YeeLightImpl::*response_callback_type)(const JsonDocumentType& response);

    void change_state(unsigned int state)
    {
        logv() << "state change from " << m_state << "(" << str_state(m_state) << ") to " << state << "(" << str_state(state) << ")";
        m_state = state;
    }

    void close_connection()
    {
        common_panic_assert(m_state == S_connecting || m_state == S_ready || m_state == S_busy);
        change_state(S_disconnecting);
        m_client.close();
        m_resp_cb = nullptr;
        m_recv_buf = std::string();
    }

    void send_request(const char *method, JsonDocumentType &request, response_callback_type cb)
    {
        common_panic_assert(cb != nullptr);
        request["id"] = 2333;
        request["method"] = method;
        if (!request.containsKey("params"))
        {
            request.createNestedArray("params");
        }
        if (m_state == S_busy)
        {
            m_next_request = request;
            m_next_resp_cb = cb;
            return;
        }
        common_panic_assert(m_state == S_ready);
        common_panic_assert(m_resp_cb == nullptr);
        m_resp_cb = cb;
        m_last_active = millis();
        change_state(S_busy);
        send_request_final(request);
    }

    void send_request_final(JsonDocument &request)
    {
        String data;
        size_t len = 0;
        serializeJson(request, data);
        len += m_client.add(data.c_str(), data.length());
        len += m_client.add("\r\n", 2);
        common_panic_assert(len == data.length() + 2);
        logd() << "send " << data;
        if (!m_client.send()) {
            logw() << "send fail, close socket";
            close_connection();
        }
    }

    void on_connect()
    {
        logi() << "connected to " << m_client.remoteIP();
        common_panic_assert(m_state == S_connecting);
        change_state(S_ready);
        m_last_active = millis();
        m_night_mode = false;
    }

    void on_disconnect()
    {
        logi() << "disconnected";
        if (m_state != S_stop)
            m_state = S_disconnect;
    }

    void on_data(const void *data, size_t len)
    {
        logv() << __func__ << " call len: " << len;
        common_panic_assert(m_state == S_ready || m_state == S_busy);
        m_last_active = millis();
        m_recv_buf += std::string(reinterpret_cast<const char*>(data), len);
        size_t pos = 0, next;
        while (1)
        {
            next = m_recv_buf.find('\r', pos);
            if (next == m_recv_buf.npos)
                break;
            if (next == m_recv_buf.length() - 1)
                break;
            if (m_recv_buf[next+1] != '\n')
            {
                logw() << "protocol error: cannot found \\n";
                close_connection();
                break;
            }
            std::string frame = m_recv_buf.substr(pos, next - pos);
            JsonDocumentType doc;
            logd() << "recv " << frame.c_str();
            if (deserializeJson(doc, frame.c_str()))
            {
                logw() << "parse fail: " << frame.c_str();
                close_connection();
                return;
            }
            else
            {
                on_data_json(doc); // be care of exception
            }
            pos = next + 2;
        }
        m_recv_buf = m_recv_buf.substr(pos);
    }

    void on_data_json(const JsonDocumentType& data)
    {
        if (data.containsKey("id"))
        {
            common_panic_assert(m_resp_cb != nullptr);
            // common_panic_assert(data.containsKey("result") && data["result"].is<JsonArray>());
            change_state(S_ready);
            (this->*m_resp_cb)(data);
            m_resp_cb = nullptr;
            if (m_next_request.size())
            {
                m_resp_cb = m_next_resp_cb;
                m_next_resp_cb = nullptr;
                change_state(S_busy);
                send_request_final(m_next_request);
                m_next_request.clear();
            }
        }
    }

    void send_heartbeat()
    {
        JsonDocumentType request;
        send_request("get_prop", request, &YeeLightImpl::cb_heartbeat);
    }

    void cb_heartbeat(const JsonDocumentType& response)
    {
        logd() << "got heartbeat";
    }

    void cb_donot_care(const JsonDocumentType& response)
    {
        logd() << "get response, don't care";
    }

private:
    int m_state;
    unsigned long m_last_active;
    bool m_night_mode;
    AsyncClient m_client;
    IPAddress m_remote;
    std::string m_recv_buf;
    JsonDocumentType m_next_request;
    response_callback_type m_next_resp_cb;
    response_callback_type m_resp_cb {nullptr};
};

static inline YeeLightImpl* as_yeelight_impl(TaskImpl *impl)
{
    return static_cast<YeeLightImpl*>(impl);
}

YeeLightTask::YeeLightTask() : Task(new YeeLightImpl)
{
}

YeeLightTask::~YeeLightTask()
{
    delete as_yeelight_impl(m_impl);
}


void YeeLightTask::set_remote(const IPAddress& remote)
{
    as_yeelight_impl(m_impl)->set_remote(remote);
}

void YeeLightTask::bright_adjust(int val)
{

}

void YeeLightTask::ct_adjust(int val)
{

}

void YeeLightTask::power_toggle()
{
    as_yeelight_impl(m_impl)->power_toggle();
}

void YeeLightTask::mode_toggle()
{
    as_yeelight_impl(m_impl)->mode_toggle();
}
