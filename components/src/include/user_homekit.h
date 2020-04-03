#ifndef __USER_HOMEKIT___
#define __USER_HOMEKIT___

#include "stdbool.h"

char *device_name_get(void);
void (*led_switch_event_update)(bool onoff, int brightness);
void (*led_brightness_event_update)(int brightness);
void user_homekit_server_init(void);

void set_light_switch(bool onoff);
void set_light_brightness(int brightness);

bool remove_homekit_pair(void);

void homekit_event_bind_handle (
        void (*light_switch_event_handle) (bool, int),
        void (*light_brightness_event_handle) (int) );
#endif