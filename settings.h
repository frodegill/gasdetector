#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>


void handleNotFound();
void handleSetupRoot();

class Settings
{
public:
  static constexpr const char* SETUP_SSID = "sensor-setup";
  static constexpr byte  EEPROM_INITIALIZED_MARKER = 0xF1; //Just a magic number

  static constexpr const byte MAX_SSID_LENGTH            = 32;
  static constexpr const byte MAX_PASSWORD_LENGTH        = 64;
  static constexpr const byte MAX_ACTIVE_SENSORS_LENGTH  =  1;
  static constexpr const byte MAX_MQTT_SERVERNAME_LENGTH = 64;
  static constexpr const byte MAX_MQTT_SENSORID_LENGTH   =  8;
  static constexpr const byte MAX_MQTT_USERNAME_LENGTH   = 32;
  static constexpr const byte MAX_MQTT_PASSWORD_LENGTH   = 32;

public:
  Settings();
  void enable();
  void readPersistentString(char* s, int max_length, int& adr);
  void readPersistentByte(uint8_t& b, int& adr);
  void readPersistentParams();
  void writePersistentString(const char* s, size_t max_length, int& adr);
  void writePersistentByte(uint8_t b, int& adr);
  void writePersistentParams(const char* ssid, const char* password);
  void handleNotFound();
  void handleSetupRoot();
  void activateSetupAP();
  void activateWifi();
  void processNetwork(bool setup_mode);
  
public:
  std::shared_ptr<ESP8266WebServer> server;

  //For SETUP_SSID AP
  DNSServer dnsServer;
  static constexpr const byte DNS_PORT = 53;
  std::shared_ptr<IPAddress> apIP;

  char ssid_param[MAX_SSID_LENGTH+1];
  char password_param[MAX_PASSWORD_LENGTH+1];
  char mqtt_servername_param[MAX_MQTT_SERVERNAME_LENGTH+1];
  char mqtt_sensorid_param[MAX_MQTT_SENSORID_LENGTH+1];
  char mqtt_username_param[MAX_MQTT_USERNAME_LENGTH+1];
  char mqtt_password_param[MAX_MQTT_PASSWORD_LENGTH+1];
};

#endif // _SETTINGS_H_
