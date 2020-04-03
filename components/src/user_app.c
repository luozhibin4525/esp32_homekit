#include "user_app.h"

#define TAG "user_app"
// const wifi_conf_t wifi_config = {
//     .mode = SC_MODE,
//     .compelete_cb = on_wifi_ready,
// };
static light_ctl_t light_t = {
    .power = 0,
    .brightness = 4000
};

/* 填充需要配置的按键个数以及对应的相关参数 */
static key_config_t gs_m_key_config[BOARD_BUTTON_COUNT] =
    {
        {BOARD_BUTTON, APP_KEY_ACTIVE_HIGH, 0, LONG_PRESSED_TIMER},
};

/** 
 * 用户的短按处理函数
 * @param[in]   key_num                 :短按按键对应GPIO口
 * @param[in]   short_pressed_counts    :短按按键对应GPIO口按下的次数,这里用不上
 * @retval      null
 * @par         修改日志 
 *               Ver0.0.1:
 */
static void short_pressed_cb(uint8_t key_num, uint8_t *short_pressed_counts)
{
  static uint8_t s_city_select = 0;
  int brightness_tmp = 0;
  switch (key_num)
  {
  case BOARD_BUTTON:
    switch (*short_pressed_counts)
    {
    case 1:
        ESP_LOGI("short_pressed_cb", "first press!!!\n");
        if(light_t.power) 
        {
            light_t.power = 0;
            set_led_brightness(0);
        }
        else 
        {
            light_t.power = 1;
            //light_t.brightness = 4000;      //50% brightness
            set_led_brightness(light_t.brightness);
        }
        /* 更新homekit数据 */
        brightness_tmp = light_t.brightness * LED_BRIGHTNESS_MAX / LED_MAX_DUTY ;
        ESP_LOGI("short_pressed_cb", "brightness_tmp %d !!\n",brightness_tmp);
        set_light_switch(light_t.power);
        set_light_brightness(brightness_tmp);

      break;
    case 2:
      ESP_LOGI("short_pressed_cb", "double press!!!\n");
      break;
    case 3:
      ESP_LOGI("short_pressed_cb", "trible press!!!\n");
      break;
    case 4:
      ESP_LOGI("short_pressed_cb", "quatary press!!!\n");
      remove_homekit_pair();
      esp_restart();
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
 *              Ver0.0.1:
                初始化版本\n 
 */
static void long_pressed_cb(uint8_t key_num, uint8_t *long_pressed_counts)
{
  static uint8_t s_press_count;
  int brightness_tmp = 0;
  switch (key_num)
  {
  case BOARD_BUTTON:
    ESP_LOGI("long_pressed_cb", "long press!!!\n");
    if(light_t.power) {
        if(s_press_count){
            /*全亮*/
            light_t.brightness = LED_MAX_DUTY;
            set_led_brightness_fade_time(light_t.brightness, 3000);
        }

        else{
            /*变暗*/
            light_t.brightness = 100;
            set_led_brightness_fade_time(light_t.brightness, 3000);
        }

        s_press_count ^= 1;
        
        /* 更新homekit数据 */
        brightness_tmp = light_t.brightness * LED_BRIGHTNESS_MAX/ LED_MAX_DUTY ;
        set_light_switch(light_t.power);
        set_light_brightness(brightness_tmp); //100 8000
    }
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
                 初始化版本\n 
 */
static void user_app_key_init(void)
{
    int32_t err_code;
    err_code = user_key_init(gs_m_key_config, BOARD_BUTTON_COUNT, DECOUNE_TIMER, long_pressed_cb, short_pressed_cb);
    ESP_LOGI("user_app_key_init", "user_key_init is %d\n", err_code);
}


void light_brightness_event_update(int brightness)
{
    
    light_t.brightness = brightness * LED_MAX_DUTY / LED_BRIGHTNESS_MAX;
    ESP_LOGI(TAG, "light_t.brightness %d",light_t.brightness);

}

void light_switch_event_update(bool onoff, int brightness)
{
    light_t.power = onoff;
    light_t.brightness = brightness * LED_MAX_DUTY / LED_BRIGHTNESS_MAX;
    
    ESP_LOGI(TAG, "light_switch_event_update light_t.brightness %d",light_t.brightness);

    if(light_t.power) {
        set_led_brightness(light_t.brightness);  
    }else {
        set_led_brightness(0);  
    }
}

/* homekit serve 初始化 */
void on_wifi_ready() {
    //homekit_server_init(&config);
    //user_homekit_server_init();
    homekit_event_bind_handle(light_switch_event_update, light_brightness_event_update);
}

/** 
 * wifi事件回调函数
 * @param[in]   null 
 * @retval      null
 * @par         修改日志 
 *               Ver0.0.1:
                 初始化版本\n 
 */
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

/** 
 * wifi初始化
 * @param[in]   null 
 * @retval      null
 * @par         修改日志 
 *               Ver0.0.1:
                 初始化版本\n 
 */
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

/** 
 * 用户应用层初始化函数
 * @param[in]   null 
 * @retval      null
 * @par         修改日志 
 *               Ver0.0.1:
                 初始化版本\n 
 */
void user_app_init(void)
{
    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    //初始化led
    led_init();
    //初始化按键
    user_app_key_init();
    //初始化wifi
    wifi_init();

}