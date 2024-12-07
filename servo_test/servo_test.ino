#include <Servo.h> 

Servo myServo;
int servoPin = 2;

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);

  // 测试舵机的最小位置
  Serial.println("Moving to minimum position...");
  myServo.writeMicroseconds(500); // 最小脉冲宽度，可能接近舵机左极限
  delay(2000);

  // 测试舵机的最大位置
  Serial.println("Moving to maximum position...");
  myServo.writeMicroseconds(2500); // 最大脉冲宽度，可能接近舵机右极限
  delay(2000);
}

void loop() {
  myServo.writeMicroseconds(500);  // 最小位置
  delay(1000);
  myServo.writeMicroseconds(2500); // 最大位置
  delay(1000);
}
