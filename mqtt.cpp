#include "mqtt.h"

#include "global.h"


void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  g_mqtt.mqttCallback(topic, payload, length);  
}


MQTT::MQTT()
{
  mqtt_client = std::make_shared<PubSubClient>(esp_client);
}

void MQTT::mqttCallback(char* topic, byte* payload, unsigned int length)
{
  g_debug.print((String("mqttCallback: ")+String(topic)+String(" value ")+String(reinterpret_cast<char*>(payload))).c_str());
}

void MQTT::publishMQTTValue(const String& topic, const String& msg)
{
  if (mqtt_enabled && connectMQTT())
  {
    bool ret = mqtt_client->publish((String(g_settings.mqtt_sensorid_param)+topic).c_str(), msg.c_str(), true);
    g_debug.print((String("publishMQTTValue topic=")+String(g_settings.mqtt_sensorid_param)+topic+String(" msg=")+msg+String(" returned ")+String(ret?"true":"false")).c_str());
  }
}

void MQTT::publishMQTTValue(const String& topic, float value)
{
  publishMQTTValue(topic, String(value, 4));
}

void MQTT::publishMQTTValues(const float* g_temperature, const float* g_humidity, const float* g_mq2_value, const float* g_mq7_value, const float* g_mq8_value, const float* g_mq9_value, const float* g_mq135_value)
{
  if (!mqtt_enabled)
    return;
}

bool MQTT::connectMQTT()
{
  byte i = 0;
  while (i++<10 && mqtt_enabled && !mqtt_client->connected())
  {
    if (mqtt_client->connect(g_settings.mqtt_sensorid_param, g_settings.mqtt_username_param, g_settings.mqtt_password_param))
    {
      g_debug.print("MQTT connected");

      g_debug.print("MQTT topics subscribed");
    }
    else
    {
      g_debug.print("MQTT waiting for reconnect");
      // Wait 3 seconds before retrying
      delay(3000);
    }
  }
  return mqtt_client->connected();
}

bool MQTT::isRequested()
{
  return g_settings.mqtt_servername_param && *g_settings.mqtt_servername_param;
}

void MQTT::initialize()
{
  mqtt_client->setServer(g_settings.mqtt_servername_param, 1883);
  mqtt_client->setCallback(::mqttCallback);
  connectMQTT();
}

void MQTT::loop()
{
  mqtt_client->loop();
}
