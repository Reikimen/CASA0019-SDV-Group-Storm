#include <Wire.h>
#include <Servo.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

#define SECRET_SSID "CE-Hub-Student"
#define SECRET_PASS "casa-ce-gagarin-public-service"
// #define SECRET_SSID "Reiki_Desktop"
// #define SECRET_PASS "Reikimen"
#define SECRET_MQTTUSER "student"
#define SECRET_MQTTPASS "ce2021-mqtt-forget-whale";

// WiFi information
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;

// MQTT server information
const char* mqtt_user = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = "mqtt.cetools.org";
const int mqtt_port       = 1884;

// MQTT Topics
const char* topic1 = "UCL/OPS/Garden/WST/dvp2/outTemp_C";
const char* topic2 = "UCL/OPS/Garden/WST/dvp2/outHumidity";
const char* topic3 = "UCL/OPS/Garden/WST/dvp2/windSpeed_kph";

WiFiClient espClient;
PubSubClient client(espClient);

Servo myServo;
int servoPin = 2;

LiquidCrystal_I2C lcd(0x27, 20, 4);

float OutTemp_C = 0;
float outHumidity = 0;
float WindSpeed_kph = 0;
float waterVaporPressure = 0;
float apparentTemperature = 0;
float Apparent_Index = 0;
int servo_angle = 0;

// The callback function when the message is received
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");

  // Converts the received message to a string
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Process messages according to different topics
  if (String(topic) == topic1) {
    Serial.println("Processing OutTemp_C...");
    Serial.println("OutTemp_C: " + message);
    // This is where the value of outTemp_C is resolved
    OutTemp_C = message.toFloat();
  } else if (String(topic) == topic2) {
    Serial.println("Processing outHumidity...");
    Serial.println("outHumidity: " + message);
    // This is where the value of outHumidity is resolved
    outHumidity = message.toFloat();
  } else if (String(topic) == topic3) {
    Serial.println("Processing WindSpeed_kph...");
    Serial.println("WindSpeed_kph: " + message);
    // This is where the value of WindSpeed_kph is resolved
    WindSpeed_kph = message.toFloat();
  } else {
    // Serial.println("Unknown topic!");
  }

  // Calculate the Water Vapor Pressure and Apparent Temperature
  // Calculate water vapor pressure
  waterVaporPressure = (outHumidity / 100) * 6.105 * exp((17.27 * OutTemp_C) / (237.7 + OutTemp_C));
  Serial.print("The water vapor pressure: ");
  Serial.println(waterVaporPressure);  // 2 specifies the number of decimal places


  // Calculate apparent temperature
  apparentTemperature = (1.04 * OutTemp_C) + (0.2 * waterVaporPressure) - (0.65 * WindSpeed_kph) - 2.7;
  Serial.print("The Apparent Temperature: ");
  Serial.println(apparentTemperature);  // 2 specifies the number of decimal places

  // % Define the variables
  // OutTemp_C = 38.1;      % Outside temperature in Celsius
  // outHumidity = 100;    % Relative humidity in percentage
  // WindSpeed_kph = 0;  % Wind speed in kilometers per hour
  // Apparent Temperature: 50.19

  // % Define the variables
  // OutTemp_C = -9.2;      % Outside temperature in Celsius
  // outHumidity = 0;    % Relative humidity in percentage
  // WindSpeed_kph = 60;  % Wind speed in kilometers per hour
  // Apparent Temperature: -54.48

  // % Define the variables, comfortable
  // OutTemp_C = 21;      % Outside temperature in Celsius
  // outHumidity = 50;    % Relative humidity in percentage
  // WindSpeed_kph = 0.2;  % Wind speed in kilometers per hour
  // Apparent Temperature: 21.49 -> 21.5

  // Thus we get the highest, lowest and most comfortable possible body surface temperature (21.5 degrees)
  // If we consider 5 degrees above and below the most comfortable temperature as the zone of moderate dressing, i.e. from 16.5 to 26.5 degrees, and 30 degrees above and below the most comfortable temperature, -11.5 to 51.5 degrees
  // Calculate the index corresponding to Apparent Temperature (from 0-90): Apparent_Index
  Apparent_Index = (apparentTemperature+11.5)*1.5;
  if (Apparent_Index < 0) Apparent_Index = 0;
  if (Apparent_Index > 90) Apparent_Index = 90;
  Serial.print("The Apparent Index: ");
  Serial.println(Apparent_Index);  // 2 specifies the number of decimal places

  // calculate the servo angle for index display
  servo_angle = int(Apparent_Index*2);

}

void LCD_fixed_display(){
  lcd.setCursor(0, 0);lcd.print("OPS Outdoor attire? ");
  lcd.setCursor(0, 1);lcd.print("LOW: PLS Wear a lot!");
  lcd.setCursor(0, 2);lcd.print("MEDIUM: Wear casual~");
  lcd.setCursor(0, 3);lcd.print("HIGH: Dress cool!   ");
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  // Reconnect to the MQTT server
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to topics
      client.subscribe("UCL/OPS/Garden/WST/dvp2/outTemp_C");
      client.subscribe("UCL/OPS/Garden/WST/dvp2/outHumidity");
      client.subscribe("UCL/OPS/Garden/WST/dvp2/windSpeed_kph");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  myServo.attach(servoPin);
  myServo.write(0);
  lcd.init();
  lcd.backlight();
  LCD_fixed_display();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  myServo.write(servo_angle);
}
