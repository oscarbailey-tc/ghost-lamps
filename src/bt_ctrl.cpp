#include <Arduino.h>
#include <BluetoothSerial.h>
#include "bt_ctrl.h"

extern BluetoothSerial SerialBT;
bool bluetooth_disconnect;

void bt_setup() {
  //SerialBT.begin(BLUETOOTH_NAME);
}

void bt_disconnect() {
  bluetooth_disconnect = true;
}

void disconnect_bluetooth()
{
  delay(1000);
  Serial.println("BT stopping");
  SerialBT.println("Bluetooth disconnecting...");
  delay(1000);
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println("BT stopped");
  delay(1000);
  bluetooth_disconnect = false;
}

void bt_loop() {
  if (bluetooth_disconnect)
  {
    disconnect_bluetooth();
  }
}
