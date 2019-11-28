#include "global.h"

#define I_SETUP_MODE_PIN               (D9) //built-in LED


Settings g_settings;
MQTT g_mqtt;
Debug g_debug;

boolean setup_mode;


void setup() {
  g_debug.enable();
  g_settings.enable();

  pinMode(I_SETUP_MODE_PIN, INPUT_PULLUP);
  delay(100);

  setup_mode = /*true ||*/ LOW == digitalRead(I_SETUP_MODE_PIN);
  if (setup_mode)
  {
    g_debug.print("Mode:Setup");
    g_settings.activateSetupAP();
  }
  else
  {
    g_debug.print("Mode:Normal");
    g_settings.activateWifi();
  }
}

void loop() {
  g_settings.processNetwork(setup_mode);
}
