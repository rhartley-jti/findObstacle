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
  Wire.begin(); //compass and gyro
  
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
  //button.waitForButton();
  delay(500);
  //button.waitForButton();
  //DoCalibrate();
  button.waitForButton();
  Serial.println("Ready for action?");
  buzzer.play(">g32>>c32");
  delay(500);
}

void SetMotorsReadSensors(struct ZumoState* s){
  gyro.read();
  compass.read();
  s->ax = compass.a.x;
  s->ay = compass.a.y;
  s->az = compass.a.z;
  s->wx = gyro.g.x;
  s->wy = gyro.g.y;
  s->wz = gyro.g.z;
  
  /*
  snprintf(report, sizeof(report), "%4d,%4d, %6d ax,%6d ay,%6d az, %6d wx,%6d wy,%6d wz",
    s->left, s->right,
    s->ax, s->ay, s->az,
    s->wx, s->wy, s->wz);
    */
    snprintf(report, sizeof(report), "%6d ax,%6d ay",
    s->ax, s->ay);
  Serial.println(report);
}

char ObstacleDetected(struct ZumoState* s){
  //acceleration
  if (-14500 > s->ax || s->ax > 6500){
    if (s->ax > 0){
      return 0;
    }
    else
      return 1;
  }
  if (-2800 > s->ay || s->ay > 1900){
    if (s->ay > 0){
      return 2;
    }
    return 3;
  }
  if (12600 > s->az || s->az > 24600){
    //return true;
  }
  return -1;
}

void loop() {
  struct ZumoState s;  
  
  
  SetMotorsReadSensors(&s);
  delay(500);
  motors.setSpeeds(200, 200);
  delay(500);
  char d = -1;
  do{
    SetMotorsReadSensors(&s);
    delay(20);
    d = ObstacleDetected(&s);
  } while (d == -1);
  if (d == 0)
  {
    motors.setSpeeds(-400, 400);
   delay(200); 
  }
  if (d == 1)
  {
   motors.setSpeeds(-200, -200);
  delay(100); 
  }
  if (d == 3)
  {
   motors.setSpeeds(-100, 100);
  delay(50); 
  }
  if (d == 2)
  {
   motors.setSpeeds(100, -100);
  delay(50); 
  }
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
  for (int i = 0; i < 20; i++){
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
