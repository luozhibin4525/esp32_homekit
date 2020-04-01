#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include <driver/gpio.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// #include "smartconfig.h"
#include "key.h"

void on_wifi_ready();

// const wifi_conf_t wifi_config = {
//     .mode = SC_MODE,
//     .compelete_cb = on_wifi_ready,

// };

/*
===========================
全局变量
=========================== 
*/
/* 填充需要配置的按键个数以及对应的相关参数 */
static key_config_t gs_m_key_config[BOARD_BUTTON_COUNT] =
    {
        {BOARD_BUTTON, APP_KEY_ACTIVE_HIGH, 0, LONG_PRESSED_TIMER},
};


homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "SPHome");


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

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            printf("STA start\n");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            printf("WiFI ready\n");
            on_wifi_ready();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            printf("STA disconnected\n");
            esp_wifi_connect();
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init() {
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "TP-LINK_BAA6",
            .password = "12344321",
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

const int led_gpio = 2;
bool led_on = false;

void led_write(bool on) {
    gpio_set_level(led_gpio, on ? 1 : 0);
}

void led_init() {
    gpio_set_direction(led_gpio, GPIO_MODE_OUTPUT);
    led_write(led_on);
}

void led_identify_task(void *_args) {
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            led_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            led_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led_write(led_on);

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
    led_write(led_on);
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
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=led_on_get,
                .setter=led_on_set,
            ),
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

void on_wifi_ready() {
    homekit_server_init(&config);
}
/** 
 * 用户的短按处理函数
 * @param[in]   key_num                 :短按按键对应GPIO口
 * @param[in]   short_pressed_counts    :短按按键对应GPIO口按下的次数,这里用不上
 * @retval      null
 * @par         修改日志 
 *               Ver0.0.1:
                     Helon_Chan, 2018/06/16, 初始化版本\n 
 */
static void short_pressed_cb(uint8_t key_num, uint8_t *short_pressed_counts)
{
  static uint8_t s_sigle_click_num = 0;
  static uint8_t s_city_select = 0;
  switch (key_num)
  {
  case BOARD_BUTTON:
    switch (*short_pressed_counts)
    {
    case 1:
      ESP_LOGI("short_pressed_cb", "first press!!!\n");
      switch (s_sigle_click_num)
      {
      case 0:
        led_write(1);
        led_on = 1;

        break;
      case 1:
        led_write(0);
        led_on = 0;
        break;
      }
      led_on_get();
      s_sigle_click_num ^= 1;

      break;
    case 2:
      ESP_LOGI("short_pressed_cb", "double press!!!\n");
      break;
    case 3:
      ESP_LOGI("short_pressed_cb", "trible press!!!\n");
      break;
    case 4:
      ESP_LOGI("short_pressed_cb", "quatary press!!!\n");
      break;
      // case ....:
      // break;
    }
    *short_pressed_counts = 0;
    break;

  default:
    break;
  }
}

/** 
 * 用户的长按处理函数
 * @param[in]   key_num                 :短按按键对应GPIO口
 * @param[in]   long_pressed_counts     :按键对应GPIO口按下的次数,这里用不上
 * @retval      null
 * @par         修改日志 
 *               Ver0.0.1:
                     Helon_Chan, 2018/06/16, 初始化版本\n 
 */
static void long_pressed_cb(uint8_t key_num, uint8_t *long_pressed_counts)
{
  switch (key_num)
  {
  case BOARD_BUTTON:
    ESP_LOGI("long_pressed_cb", "long press!!!\n");
    break;
  default:
    break;
  }
}

/** 
 * 用户的按键初始化函数
 * @param[in]   null 
 * @retval      null
 * @par         修改日志 
 *               Ver0.0.1:
                     Helon_Chan, 2018/06/16, 初始化版本\n 
 */
static void user_app_key_init(void)
{
  int32_t err_code;
  err_code = user_key_init(gs_m_key_config, BOARD_BUTTON_COUNT, DECOUNE_TIMER, long_pressed_cb, short_pressed_cb);
  ESP_LOGI("user_app_key_init", "user_key_init is %d\n", err_code);
}


void app_main(void) {
    // Initialize NVS
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK( ret );
    ESP_ERROR_CHECK(nvs_flash_init());
    name.value = HOMEKIT_STRING(device_name_get());

    wifi_init();
    //initialise_wifi(&wifi_config);
    led_init();
    user_app_key_init();
}
