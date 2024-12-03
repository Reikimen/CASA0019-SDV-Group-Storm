#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// #define SECRET_SSID "CE-Hub-Student"
// #define SECRET_PASS "casa-ce-gagarin-public-service"
#define SECRET_SSID "Reiki_Desktop"
#define SECRET_PASS "Reikimen"
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
const char* topic2 = "UCL/OPS/Garden/WST/dvp2/inTemp_C";
const char* topic3 = "UCL/OPS/Garden/WST/dvp2/windSpeed_kph";

WiFiClient espClient;
PubSubClient client(espClient);

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
  } else if (String(topic) == topic2) {
    Serial.println("Processing InTemp_C...");
    Serial.println("InTemp_C: " + message);
    // This is where the value of InTemp_C is resolved
  } else if (String(topic) == topic3) {
    Serial.println("Processing WindSpeed_kph...");
    Serial.println("WindSpeed_kph: " + message);
    // This is where the value of WindSpeed_kph is resolved
  } else {
    // Serial.println("Unknown topic!");
  }
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
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
