#ifndef WIFI_CTRL_H
#define WIFI_CTRL_H

#include "BluetoothSerial.h"

void wifi_setup();
void wifi_disconnect();
bool wifi_test_connection();

#endif // WIFI_CTRL_H
