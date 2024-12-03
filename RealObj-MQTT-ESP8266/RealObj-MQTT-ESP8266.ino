#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// #define SECRET_SSID "CE-Hub-Student"
// #define SECRET_PASS "casa-ce-gagarin-public-service"
#define SECRET_SSID "Reiki_Desktop"
#define SECRET_PASS "Reikimen"
#define SECRET_MQTTUSER "student"
#define SECRET_MQTTPASS "ce2021-mqtt-forget-whale";

// WiFi信息
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;

// MQTT服务器信息
const char* mqtt_user = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = "mqtt.cetools.org";
const int mqtt_port       = 1884;

WiFiClient espClient;
PubSubClient client(espClient);

// 接收到消息时的回调函数
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");

  // 将接收到的消息转换为字符串
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Payload: " + message);

  // 解析JSON
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("JSON Parsing failed: ");
    Serial.println(error.f_str());
    return;
  }

  // 你可以根据主题名来区分不同的数据处理逻辑
  if (String(topic) == "UCL/OPS/Garden/WST/dvp2/outTemp_C") {
    // 处理outTemp_C的逻辑
    float outTemp_C = doc["outTemp_C"];
    Serial.print("outTemp_C: ");
    Serial.println(outTemp_C);
  }
  else if (String(topic) == "UCL/OPS/Garden/WST/dvp2/outHumidity") {
    // 处理 outHumidity 的逻辑
    float outHumidity = doc["outHumidity"];
    Serial.print("outHumidity: ");
    Serial.println(outHumidity);
  }
  else if (String(topic) == "UCL/OPS/Garden/WST/dvp2/windSpeed_kph") {
    // 处理windSpeed_kph的逻辑
    float windSpeed_kph = doc["windSpeed_kph"];
    Serial.print("windSpeed_kph: ");
    Serial.println(windSpeed_kph);
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
  // 重新连接MQTT服务器
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // 订阅主题
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
