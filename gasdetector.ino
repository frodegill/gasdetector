#include "global.h"

#include <DHTesp.h>

#define I_SETUP_MODE_PIN               (D8)
#define O_POWER_VOLT_PIN               (D2) //LOW = 5V, HIGH = 1.4V
#define O_POWER_ANALOG_PIN             (D3) //LOW = reduce (5V->3.3V), HIGH = do not reduce (1.4V->1.4V)
#define I_AM2302_PIN                   (D4)
#define O_ADDR_MASK0_PIN               (D7) // 0xxx1
#define O_ADDR_MASK1_PIN               (D6) // 0xx1x
#define O_ADDR_MASK2_PIN               (D5) // 0x1xx

static const byte MQ_2_PORT   = 0;
static const byte MQ_7_PORT   = 1;
static const byte MQ_8_PORT   = 2;
static const byte MQ_9_PORT   = 3;
static const byte MQ_135_PORT = 4;

static const byte OLD = 0;
static const byte CURRENT = 1;

enum PowerVolt {
  POWER_5_0,
  POWER_1_4
};

enum AnalogVolt {
  ANALOG_3_3,
  ANALOG_1_4
};

#define DHT_MODEL (DHTesp::AM2302)


Settings g_settings;
MQTT g_mqtt;
Debug g_debug;
DHTesp g_dht;

boolean g_setup_mode;

PowerVolt g_power_volt;
AnalogVolt g_analog_volt;

float g_temperature[2];
float g_humidity[2];
float g_mq2_value[2];
float g_mq7_value[2];
float g_mq8_value[2];
float g_mq9_value[2];
float g_mq135_value[2];

unsigned long g_previous_am2302_time;
unsigned long g_previous_mq2_time;
unsigned long g_previous_mq8_time;
unsigned long g_previous_mq135_time;

unsigned long g_toggle_power_volt_time;


void togglePowerVolt(PowerVolt power_volt)
{
  if (g_power_volt == power_volt)
  {
    return;
  }
  g_power_volt = power_volt;
  digitalWrite(O_POWER_VOLT_PIN, g_power_volt==POWER_1_4?HIGH:LOW);
  g_toggle_power_volt_time = millis();
  delay(25);
}

void toggleAnalogPower(AnalogVolt analog_volt)
{
  if (g_analog_volt == analog_volt)
  {
    return;
  }
  g_analog_volt = analog_volt;
  digitalWrite(O_POWER_ANALOG_PIN, g_analog_volt==ANALOG_1_4?HIGH:LOW);
  delay(25);
}

void selectMultiplexerPort(const byte port)
{
  digitalWrite(O_ADDR_MASK0_PIN, (port&0x01)!=0?HIGH:LOW);
  digitalWrite(O_ADDR_MASK1_PIN, (port&0x02)!=0?HIGH:LOW);
  digitalWrite(O_ADDR_MASK2_PIN, (port&0x04)!=0?HIGH:LOW);
  delay(25);
}

uint16_t averageAnalogValue()
{
  uint16_t sum;
  for (byte i=0; i<5; i++)
  {
    sum += analogRead(A0);
    delay(5);
  }
  return sum/5;
}

void setup() {
  g_debug.enable();
  g_settings.enable();

  pinMode(I_SETUP_MODE_PIN, INPUT_PULLUP);
  delay(100);

  g_setup_mode = /*true ||*/ LOW == digitalRead(I_SETUP_MODE_PIN);
  if (g_setup_mode)
  {
    g_debug.print("Mode:Setup");
    g_settings.activateSetupAP();
  }
  else
  {
    g_debug.print("Mode:Normal");
    g_settings.activateWifi();

    pinMode(O_POWER_VOLT_PIN, OUTPUT);
    pinMode(O_POWER_ANALOG_PIN, OUTPUT);
    pinMode(O_ADDR_MASK0_PIN, OUTPUT);
    pinMode(O_ADDR_MASK1_PIN, OUTPUT);
    pinMode(O_ADDR_MASK2_PIN, OUTPUT);

    g_dht.setup(I_AM2302_PIN, DHT_MODEL);
    g_temperature[OLD] = 0;
    g_humidity[OLD] = 0;
    g_previous_am2302_time = g_previous_mq2_time = g_previous_mq8_time = g_previous_mq135_time = millis();

    g_power_volt = POWER_1_4; //To force update in next line
    togglePowerVolt(POWER_5_0);
    
    g_analog_volt = ANALOG_3_3; //To force update in next line
    toggleAnalogPower(ANALOG_1_4);
  }
}

void loop() {
  g_settings.processNetwork(g_setup_mode);

  unsigned long now;
  
  //AM2302
  now = millis();
  if (now>=(g_previous_am2302_time+g_dht.getMinimumSamplingPeriod() ||
      now<g_previous_am2302_time)) // millis() will wrap every ~50 days
  {
    TempAndHumidity temp_and_humidity = g_dht.getTempAndHumidity();
    if (g_dht.getStatus() == DHTesp::ERROR_NONE)
    {
      g_temperature[CURRENT] = temp_and_humidity.temperature;
      g_humidity[CURRENT] = temp_and_humidity.humidity;
      g_debug.print((String("reading temp ")+String(g_temperature[CURRENT], 1)+String(" and humidity ")+String(g_humidity[CURRENT], 1)).c_str());
      g_previous_am2302_time = now;
    }
    else
    {
      g_debug.print((String("reading temp and humidity failed with error ")+String((int)g_dht.getStatus())).c_str());
    }
  }

  //MQ-7/MQ-9
  now = millis();
  if (g_power_volt==POWER_5_0 &&
      (now>=(g_toggle_power_volt_time+90*1000) ||
       now<g_toggle_power_volt_time)) // millis() will wrap every ~50 days
  {
    togglePowerVolt(POWER_1_4);
  }
  else if (g_power_volt==POWER_5_0 &&
           (now>=(g_toggle_power_volt_time+60*1000) ||
            now<g_toggle_power_volt_time)) // millis() will wrap every ~50 days
  {
    toggleAnalogPower(ANALOG_1_4);

    selectMultiplexerPort(MQ_7_PORT);
    g_mq7_value[CURRENT] = averageAnalogValue()/1023.0f;
    
    selectMultiplexerPort(MQ_9_PORT);
    g_mq9_value[CURRENT] = averageAnalogValue()/1023.0f;

    togglePowerVolt(POWER_5_0);
  }

  //MQ-2
  now = millis();
  if (now>=(g_previous_mq2_time+20*1000) ||
      now<g_previous_mq2_time) // millis() will wrap every ~50 days
  {
    toggleAnalogPower(ANALOG_3_3);

    selectMultiplexerPort(MQ_2_PORT);
    g_mq2_value[CURRENT] = averageAnalogValue()/1023.0f;
  }

  //MQ-8
  now = millis();
  if (now>=(g_previous_mq8_time+20*1000) ||
      now<g_previous_mq8_time) // millis() will wrap every ~50 days
  {
    toggleAnalogPower(ANALOG_3_3);

    selectMultiplexerPort(MQ_8_PORT);
    g_mq8_value[CURRENT] = averageAnalogValue()/1023.0f;
  }

  //MQ-135
  now = millis();
  if (now>=(g_previous_mq135_time+20*1000) ||
      now<g_previous_mq135_time) // millis() will wrap every ~50 days
  {
    toggleAnalogPower(ANALOG_3_3);

    selectMultiplexerPort(MQ_135_PORT);
    g_mq135_value[CURRENT] = averageAnalogValue()/1023.0f;
  }

  g_mqtt.publishMQTTValues(g_temperature, g_humidity, g_mq2_value, g_mq7_value, g_mq8_value, g_mq9_value, g_mq135_value);

}
