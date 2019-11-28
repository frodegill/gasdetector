#include "settings.h"

#include "global.h"


void handleNotFound()
{
  g_settings.handleNotFound();
}

void handleSetupRoot()
{
  g_settings.handleSetupRoot();
}

Settings::Settings()
{
  server = std::make_shared<ESP8266WebServer>(80);
  apIP = std::make_shared<IPAddress>(192, 168, 4, 1);
  ssid_param[0] = password_param[0] = mqtt_servername_param[0] = mqtt_sensorid_param[0] = mqtt_username_param[0] = mqtt_password_param[0] = 0;
}

void Settings::enable()
{
  EEPROM.begin(1 + MAX_SSID_LENGTH+1 + MAX_PASSWORD_LENGTH+1 + MAX_ACTIVE_SENSORS_LENGTH + MAX_MQTT_SERVERNAME_LENGTH+1 + MAX_MQTT_SENSORID_LENGTH+1 + MAX_MQTT_USERNAME_LENGTH+1 + MAX_MQTT_PASSWORD_LENGTH+1);
}

void Settings::readPersistentString(char* s, int max_length, int& adr)
{
  int i = 0;
  byte c;
  do
  {
    c = EEPROM.read(adr++);
    if (i<max_length)
    {
      s[i++] = static_cast<char>(c);
    }
  } while (c!=0);
  s[i] = 0;
}

void Settings::readPersistentByte(uint8_t& b, int& adr)
{
  b = EEPROM.read(adr++);
}

void Settings::readPersistentParams()
{
  int adr = 0;
  if (EEPROM_INITIALIZED_MARKER != EEPROM.read(adr++))
  {
    ssid_param[0] = 0;
    password_param[0] = 0;
    mqtt_servername_param[0] = 0;
    mqtt_sensorid_param[0] = 0;
    mqtt_username_param[0] = 0;
    mqtt_password_param[0] = 0;
  }
  else
  {
    readPersistentString(ssid_param, MAX_SSID_LENGTH, adr);
    readPersistentString(password_param, MAX_PASSWORD_LENGTH, adr);
    readPersistentString(mqtt_servername_param, MAX_MQTT_SERVERNAME_LENGTH, adr);
    readPersistentString(mqtt_sensorid_param, MAX_MQTT_SENSORID_LENGTH, adr);
    readPersistentString(mqtt_username_param, MAX_MQTT_USERNAME_LENGTH, adr);
    readPersistentString(mqtt_password_param, MAX_MQTT_PASSWORD_LENGTH, adr);
  }
}

void Settings::writePersistentString(const char* s, size_t max_length, int& adr)
{
  for (int i=0; i<min(strlen(s), max_length); i++)
  {
    EEPROM.write(adr++, s[i]);
  }
  EEPROM.write(adr++, 0);
}

void Settings::writePersistentByte(uint8_t b, int& adr)
{
  EEPROM.write(adr++, b);
}

void Settings::writePersistentParams(const char* ssid, const char* password)
{
  int adr = 0;
  EEPROM.write(adr++, EEPROM_INITIALIZED_MARKER);
  writePersistentString(ssid, MAX_SSID_LENGTH, adr);
  writePersistentString(password, MAX_PASSWORD_LENGTH, adr);
  writePersistentString(mqtt_servername_param, MAX_MQTT_SERVERNAME_LENGTH, adr);
  writePersistentString(mqtt_sensorid_param, MAX_MQTT_SENSORID_LENGTH, adr);
  writePersistentString(mqtt_username_param, MAX_MQTT_USERNAME_LENGTH, adr);
  writePersistentString(mqtt_password_param, MAX_MQTT_PASSWORD_LENGTH, adr);
  EEPROM.commit();
}

void Settings::handleNotFound()
{
  server->send(404, F("text/plain"), F("Page Not Found\n"));
}

void Settings::handleSetupRoot()
{
  if (server->hasArg("ssid") || server->hasArg("password")
      || server->hasArg("mqtt_server") || server->hasArg("mqtt_id") || server->hasArg("mqtt_username") || server->hasArg("mqtt_password"))
  {
    if (server->hasArg("ssid"))
    {
      strncpy(ssid_param, server->arg("ssid").c_str(), MAX_SSID_LENGTH);
      ssid_param[MAX_SSID_LENGTH] = 0;
    }
    
    if (server->hasArg("password") && !server->arg("password").equals(F("password")))
    {
      strncpy(password_param, server->arg("password").c_str(), MAX_PASSWORD_LENGTH);
      password_param[MAX_PASSWORD_LENGTH] = 0;
    }

    if (server->hasArg("mqtt_server"))
    {
      strncpy(mqtt_servername_param, server->arg("mqtt_server").c_str(), MAX_MQTT_SERVERNAME_LENGTH);
      mqtt_servername_param[MAX_MQTT_SERVERNAME_LENGTH] = 0;
    }
    if (server->hasArg("mqtt_id"))
    {
      strncpy(mqtt_sensorid_param, server->arg("mqtt_id").c_str(), MAX_MQTT_SENSORID_LENGTH);
      mqtt_sensorid_param[MAX_MQTT_SENSORID_LENGTH] = 0;
    }
    if (server->hasArg("mqtt_username"))
    {
      strncpy(mqtt_username_param, server->arg("mqtt_username").c_str(), MAX_MQTT_USERNAME_LENGTH);
      mqtt_username_param[MAX_MQTT_USERNAME_LENGTH] = 0;
    }
    if (server->hasArg("mqtt_password") && !server->arg("mqtt_password").equals(F("mqtt_password")))
    {
      strncpy(mqtt_password_param, server->arg("mqtt_password").c_str(), MAX_MQTT_PASSWORD_LENGTH);
      mqtt_password_param[MAX_MQTT_PASSWORD_LENGTH] = 0;
    }

    writePersistentParams(ssid_param, password_param);

    server->send(200, F("text/plain"), F("Settings saved"));
  }
  else
  {
    readPersistentParams();

    String body = F("<!doctype html>"\
                    "<html lang=\"en\">"\
                    "<head>"\
                     "<meta charset=\"utf-8\">"\
                     "<title>Setup</title>"\
                     "<style>"\
                      "form {margin: 0.5em;}"\
                      "input {margin: 0.2em;}"\
                     "</style>"\
                    "</head>"\
                    "<body>"\
                     "<form method=\"post\">"\
                      "SSID:<input type=\"text\" name=\"ssid\" required maxlength=\"");
    body += String(MAX_SSID_LENGTH);
    body += F("\" autofocus value=\"");
    body += ssid_param;
    body += F("\"/><br/>"\
              "Password:<input type=\"password\" name=\"password\" maxlength=\"");
    body += String(MAX_PASSWORD_LENGTH);
    body += F("\" value=\"password\"/><br/><hr/>"\
              "MQTT server:<input type=\"text\" name=\"mqtt_server\" maxlength=\"");
    body += String(MAX_MQTT_SERVERNAME_LENGTH);
    body += F("\" value=\"");
    body += mqtt_servername_param;
    body += F("\"/><br/>"\
              "MQTT sensor id:<input type=\"text\" name=\"mqtt_id\" maxlength=\"");
    body += String(MAX_MQTT_SENSORID_LENGTH);
    body += F("\" value=\"");
    body += mqtt_sensorid_param;
    body += F("\"/><br/>"\
              "MQTT username:<input type=\"text\" name=\"mqtt_username\" maxlength=\"");
    body += String(MAX_MQTT_USERNAME_LENGTH);
    body += F("\" value=\"");
    body += mqtt_username_param;
    body += F("\"/><br/>"\
              "MQTT password:<input type=\"password\" name=\"mqtt_password\" maxlength=\"");
    body += String(MAX_MQTT_PASSWORD_LENGTH);
    body += F("\" value=\"password\"/><br/>"\
              "<input type=\"submit\" value=\"Submit\"/>"\
              "</form>"\
             "</body>"\
             "</html>");
    server->send(200, F("text/html"), body);
  }
}

void Settings::activateSetupAP()
{
  WiFi.softAP(SETUP_SSID);
  dnsServer.start(DNS_PORT, "*", *apIP.get());
  
  server->on("/", ::handleSetupRoot);
  server->onNotFound(::handleNotFound);
  server->begin();
}

void Settings::activateWifi()
{
  readPersistentParams();
  
  g_debug.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_param, password_param);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  g_debug.print("WiFi connected");

  if (g_mqtt.isRequested())
  {
    g_debug.print("Initializing MQTT");
    g_mqtt.initialize();
  }
}

void Settings::processNetwork(bool setup_mode)
{
  if (setup_mode)
  {
    dnsServer.processNextRequest(); //Route everything to 192.168.4.1
    server->handleClient(); //WebServer
  }
  else
  {
    if (g_mqtt.isRequested() && g_mqtt.connectMQTT())
    {
      g_mqtt.loop();
    }
  }
}
