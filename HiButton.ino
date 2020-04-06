#include "common.h"
#include "task.h"
#include "WiFiTask.h"
#include "YeeLight.h"
#include "ButtonTask.h"
#include "ConfigTask.h"
#include "config.h"

static WiFiTask wifi_task;
static YeeLightTask yeelight_task;
static ButtonTask button_task(D3, LOW);
static ConfigTask config_task;

void setup()
{
    common_init();

    button_task.on_click_callback([](){
        yeelight_task.power_toggle();
    });
    button_task.on_double_click_callback([](){
        yeelight_task.mode_toggle();
    });
    button_task.on_long_press_callback([](){
        wifi_task.configure_reset();
    });

    IPAddress remote;
    remote.fromString(DEVICE_ADDR);
    yeelight_task.set_remote(remote);
    config_task.on_config_wifi([](const char *ssid, const char *key){
        wifi_task.configure(ssid, key);
    });
    config_task.set_log_level(LOG_VERBOSE);

    wifi_task.start();
    yeelight_task.start();
    button_task.start();
    config_task.start();
}

void loop()
{
    wifi_task.loop();
    yeelight_task.loop();
    button_task.loop();
    config_task.loop();
}