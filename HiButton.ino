#include "common.h"
#include "task.h"
#include "WiFiTask.h"
#include "YeeLight.h"
#include "ButtonTask.h"
#include "config.h"

static WiFiTask wifi_task;
static YeeLightTask yeelight_task;
static ButtonTask button_task(D3, LOW);

void setup()
{
    common_init();

    button_task.on_click_callback([](){
        yeelight_task.power_toggle();
    });
    button_task.on_double_click_callback([](){
        yeelight_task.mode_toggle();
    });

    IPAddress remote;
    remote.fromString(DEVICE_ADDR);
    yeelight_task.set_remote(remote);

    wifi_task.start();
    yeelight_task.start();
    button_task.start();
}

void loop()
{
    wifi_task.loop();
    yeelight_task.loop();
    button_task.loop();
}