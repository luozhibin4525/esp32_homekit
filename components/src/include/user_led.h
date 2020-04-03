#ifndef __USER_LED_H__
#define __USER_LED_H__

#define LEDC_HS_TIMER           LEDC_TIMER_0
#define LEDC_HS_MODE            LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO        (2)
#define LEDC_HS_CH0_CHANNEL     LEDC_CHANNEL_0

#define LED_MAX_DUTY            8000
#define LED_BRIGHTNESS_MAX      100
#define LEDC_FADE_TIME          1000

void led_init(void);
void set_led_brightness(int brightness);
void set_led_brightness_fade_time(int brightness, int times);
#endif