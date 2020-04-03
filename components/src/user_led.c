#include "user_led.h"
#include <driver/gpio.h>
#include "driver/ledc.h"

const ledc_channel_config_t ledc_channel[1] = {
    {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    }
};

void set_led_brightness(int brightness) {
    ledc_set_fade_with_time(ledc_channel[0].speed_mode,
            ledc_channel[0].channel, brightness, LEDC_FADE_TIME);

    ledc_fade_start(ledc_channel[0].speed_mode,
            ledc_channel[0].channel, LEDC_FADE_NO_WAIT);
}

void set_led_brightness_fade_time(int brightness, int times) {
    ledc_set_fade_with_time(ledc_channel[0].speed_mode,
            ledc_channel[0].channel, brightness, times);

    ledc_fade_start(ledc_channel[0].speed_mode,
            ledc_channel[0].channel, LEDC_FADE_NO_WAIT);
}


void led_init(void) {
    // gpio_set_direction(led_gpio, GPIO_MODE_OUTPUT);
    // led_write(led_on);
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 500,                      // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,           // timer mode
        .timer_num = LEDC_HS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
 
    ledc_channel_config(&ledc_channel[0]);
    // Initialize fade service.
    ledc_fade_func_install(0);
}