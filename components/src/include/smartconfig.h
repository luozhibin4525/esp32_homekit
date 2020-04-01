#ifndef __SMARTCONFIG_H__
#define __SMARTCONFIG_H__


#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"

#define SC_MODE     0
#define AP_MODE     1

typedef void (*user_wifi_connected_callback_t)(void);


typedef struct
{
    uint8_t mode; //ap or sc
    user_wifi_connected_callback_t compelete_cb;
}wifi_conf_t;

void initialise_wifi(wifi_conf_t *config);

#endif