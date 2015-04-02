#include <EEPROM.h>
#include <Wire.h>

#include <L3G.h>
#include <LSM303.h>
#include <Pushbutton.h>

#include <ZumoBuzzer.h>
#include <ZumoMotors.h>

L3G gyro;
LSM303 compass;
Pushbutton button(ZUMO_BUTTON);
ZumoBuzzer buzzer;
ZumoMotors motors;

char report[80];

struct ZumoState{
   int left, right;
   int ax, ay, az;
   int wx, wy, wz;
};

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  compass.init();
  compass.enableDefault();

  if (!gyro.init())
  {
    Serial.println("Failed to autodetect gyro type!");
    while (1);
  }
  gyro.enableDefault();
  
  Serial.println("Ready to calibrate?");

  buzzer.play(">g32");
  button.waitForButton();
  delay(500);
  DoCalibrate();
}

void DoCalibrate(){
  struct ZumoState s;
  s.left = 0;
  s.right = 0;
  for (int i = 0; i < 10; i++){
      SetMotorsReadSensors(&s);
      delay(20);
  }
  s.left = 200;
  s.right = 200;
  for (int i = 0; i < 50; i++){
      SetMotorsReadSensors(&s);
      delay(20);
  }
  s.left = 0;
  s.right = 0;
  for (int i = 0; i < 10; i++){
      SetMotorsReadSensors(&s);
      delay(20);
  }
  s.left = -200;
  s.right = -100;
  
  for (int i = 0; i < 35; i++){
      SetMotorsReadSensors(&s);
      delay(20);
  }
  motors.setSpeeds(0,0);
}

void SetMotorsReadSensors(struct ZumoState* s){
  gyro.read();
  compass.read();
  motors.setSpeeds(s->left, s->right);
  s->ax = compass.a.x;
  s->ay = compass.a.y;
  s->az = compass.a.z;
  s->wx = gyro.g.x;
  s->wy = gyro.g.y;
  s->wz = gyro.g.z;
  
  snprintf(report, sizeof(report), "%4d,%4d, %6d,%6d,%6d, %6d,%6d,%6d",
    s->left, s->right,
    s->ax, s->ay, s->az,
    s->wx, s->wy, s->wz);
  Serial.println(report);
}

bool ObstacleDetected(struct ZumoState* s){
  return false;
}

void loop() {
  struct ZumoState s;
  s.left = 200;
  s.right = 150;
  Serial.println("Ready for action?");
  buzzer.play(">g32>>c32");
  button.waitForButton();
  delay(1000);
  do{
    SetMotorsReadSensors(&s);
    
    delay(20);
  } while ( !ObstacleDetected(&s));
}
