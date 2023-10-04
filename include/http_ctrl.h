#ifndef HTTP_CTRL_H
#define HTTP_CTRL_H

#include "color.h"

void http_loop();
bool http_update_supabase();
void upload_color(rgb_t color);

#endif // HTTP_CTRL_H
