#include <Arduino.h>
#include "serial_cmds.h"
#include "wifi_ctrl.h"

String serial_cmd;
long serial_cmd_start;

void process_cmd(String cmd) {
  cmd.toLowerCase();
  if (cmd == "forget_wifi") {
    wifi_forget();
    wifi_setup();
  }
  //if (cmd == "pinw") {
  //  Serial.println("Enter pin number and state: ");
  //  uint8_t args[2];
  //  size_t args_len = 0;
  //  while (args_len < 2) {
  //    if (Serial.available()) {
  //      char in = Serial.read();
  //      char in_str[2] = {in, '\0'};
  //      args[args_len++] = atoi(in_str);
  //    }
  //  }

  //  switch (args[0]) {
  //    case 2:
  //    case 4:
  //    case 5:
  //      digitalWrite(args[0], args[1] > 0);
  //      Serial.print("Writing pin ");
  //      Serial.print(args[0]);
  //      Serial.print(" to ");
  //      Serial.println(args[1]);
  //      break;
  //    default:
  //      Serial.print("Pin ");
  //      Serial.print(args[0]);
  //      Serial.println(" not supported");
  //      break;
  //  }
  //}
}

void handle_serial_cmd() 
{
  if (Serial.available()) {
    if (serial_cmd.length() == 0) {
      serial_cmd_start = millis();
    }
    char cur = (char) Serial.read();
    if (cur == ';') {
      process_cmd(serial_cmd);
      serial_cmd = "";
    } else {
      serial_cmd.concat(cur);
    }
  }

  if (serial_cmd.length() == 0) {
    return;
  }

  if (serial_cmd.length() > 0 && millis() - serial_cmd_start > SERIAL_CMD_TIMEOUT_MS) {
    Serial.println("Serial command timeout.");
    serial_cmd = "";
  }

  if (serial_cmd.length() > SERIAL_CMD_MAX_LENGTH) {
    Serial.println("Serial command too long, wiping buffer");
  }
}
