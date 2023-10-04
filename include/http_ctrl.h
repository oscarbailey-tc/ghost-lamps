#ifndef HTTP_CTRL_H
#define HTTP_CTRL_H

#include "color.h"

void http_loop();
bool http_update_supabase();
void http_read_lamp_group();

void set_and_upload_random_led_color();

#endif // HTTP_CTRL_H
