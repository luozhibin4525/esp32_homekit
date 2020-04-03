#include "user_homekit.h"
#include <stdio.h>
#include <esp_wifi.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "SPHome");

bool led_on = false;
int led_brightness = 50;
int led_brightness_val = 0;

char *device_name_get(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    int name_len = snprintf(NULL, 0, "SPHome-%02X%02X%02X",
                            mac[3], mac[4], mac[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "SPHome-%02X%02X%02X",
             mac[3], mac[4], mac[5]);

    return name_value;
}

void led_identify_task(void *_args) {
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            // led_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            // led_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    // led_write(led_on);

    vTaskDelete(NULL);
}

void led_identify(homekit_value_t _value) {
    printf("LED identify\n");
    xTaskCreate(led_identify_task, "LED identify", 512, NULL, 2, NULL);
}

homekit_value_t led_on_get() {
	printf("get led status %d\n",led_on);
    return HOMEKIT_BOOL(led_on);
}

void led_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    led_on = value.bool_value;
	printf("set led %d\n",led_on);
    
    if(led_switch_event_update) {
        led_switch_event_update(led_on, led_brightness);
    }
}

homekit_characteristic_t light_switch = HOMEKIT_CHARACTERISTIC_(
                ON, false,
                .getter=led_on_get,
                .setter=led_on_set,
);

/** @brief Smartf Elfin APP 控制开关 状态通知**/  
void set_light_switch(bool onoff) {
    led_on = onoff;
    homekit_characteristic_notify(&light_switch, HOMEKIT_BOOL(onoff));
}

/** @brief Home APP 获取灯亮度 **/ 
homekit_value_t led_brightness_get() {
    printf("get led brightness status %d\n",led_brightness);
    return HOMEKIT_INT(led_brightness);
}

/** @brief Home APP 控制灯亮度 **/ 
void led_brightness_set(homekit_value_t value) {
        if (value.format != homekit_format_int) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }
    led_brightness = value.int_value;
    printf("set led brightness %d\n",led_brightness);

    if(led_brightness_event_update) {
        led_brightness_event_update(led_brightness);
    }

}

/** @brief Smartf Elfin APP 控制 亮度 状态通知**/  
homekit_characteristic_t light_brightness = HOMEKIT_CHARACTERISTIC_(
    BRIGHTNESS, 50,
    .getter = led_brightness_get,
    .setter = led_brightness_set
);

/** @brief Smartf Elfin APP 亮度 状态上报**/  
void set_light_brightness(int brightness) {
    led_brightness = brightness;
    homekit_characteristic_notify(&light_brightness, HOMEKIT_INT(brightness));
}


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            &name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "PsHome"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0"),
            HOMEKIT_CHARACTERISTIC(MODEL, "MyLED"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Smart LED"),
            &light_switch,
            &light_brightness,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111",
    .setupId="1SP0",
};

void user_homekit_server_init(void)
{
    name.value = HOMEKIT_STRING(device_name_get());
    homekit_server_init(&config);
}

bool remove_homekit_pair(void) {
    if(homekit_is_paired()) {
        homekit_server_reset();
        return true;
    }
    return false;
}

void homekit_event_bind_handle (
        void (*light_switch_event_handle)(bool, int),
        void (*light_brightness_event_handle)(int) )
{
    led_switch_event_update = light_switch_event_handle;
    led_brightness_event_update = light_brightness_event_handle;
    
    user_homekit_server_init();

}