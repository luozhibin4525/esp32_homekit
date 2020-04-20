#ifndef __USER_APP_H__
#define __USER_APP_H__

#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "key.h"
#include "user_led.h"
#include "smartconfig.h"
#include "user_homekit.h"

typedef struct 
{
    bool power;
    uint16_t brightness;
}light_ctl_t;

void user_app_init(void);

#endif