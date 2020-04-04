#include "common.h"
#include "task.h"
#include "WiFiTask.h"
#include "YeeLight.h"
#include "config.h"

static WiFiTask wifi_task;
static YeeLightTask yeelight_task;

void setup()
{
    common_init();
    yeelight_task.set_log_level(LOG_VERBOSE);
    wifi_task.start();
    yeelight_task.start();
    IPAddress remote;
    remote.fromString(DEVICE_ADDR);
    yeelight_task.set_remote(remote);
}

void loop()
{
    delay(50);
    wifi_task.loop();
    yeelight_task.loop();
}