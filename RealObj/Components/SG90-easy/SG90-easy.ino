#include <Servo.h>

// 创建 Servo 对象
Servo myServo;

// 定义舵机信号引脚
const int servoPin = 2; // D4

void setup() {
  // 将舵机对象连接到指定的引脚
  myServo.attach(servoPin);
  // 初始化舵机位置为 0 度
  myServo.write(0);
  delay(1000); // 等待舵机移动到初始位置
}

void loop() {
  // 循环摆动舵机
  // for (int i = 0; i<180; i++){
  //   myServo.write(i);
  //   delay(30); // 控制舵机移动速度
  // }
  // for (int j = 180; j>=0; j--){
  //   myServo.write(j);
  //   delay(30); // 控制舵机移动速度
  // }
  myServo.write(50);
  delay(1000);

}
