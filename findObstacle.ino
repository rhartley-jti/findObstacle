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
const int LOG_COLUMNS = 8;
const int LOG_ROWS = 100;
const int LOG_BIAS = 32768;

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
  
  //send data from last run to serial
  
  int addr = 0;
  for (int i = 0; i < LOG_ROWS; i++){ 
    for (int j = 0; j < LOG_COLUMNS; j++){
      int val;
      val += EEPROM.read(addr++);
      val *= 256;
      val += EEPROM.read(addr++);
      val -= LOG_BIAS;
      addr += sizeof(int);
      Serial.print(val);
      Serial.print(" ");
    }
    Serial.println("");
  }

  buzzer.play(">g32");
  button.waitForButton();
  delay(500);
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
  buzzer.play(">g32>>c32");
  button.waitForButton();
  delay(1000);
}


int logAddr = 0;
int logRow = 0;

void put(int val){
  EEPROM.write(logAddr++, (val+LOG_BIAS)/256);
  EEPROM.write(logAddr++, (val+LOG_BIAS)%256);
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
  if (logRow < LOG_ROWS){
      ++logRow;
      put(s->left);
      put(s->right);
      put(compass.a.x);
      put(compass.a.y);
      put(compass.a.z);
      put(gyro.g.x);
      put(gyro.g.y);
      put(gyro.g.z);
  }
  
  snprintf(report, sizeof(report), "%4d,%4d, %6d,%6d,%6d, %6d,%6d,%6d",
    s->left, s->right,
    s->ax, s->ay, s->az,
    s->wx, s->wy, s->wz);
  Serial.println(report);
}

void loop() {
  struct ZumoState s;
  s.left = 200;
  s.right = 150;
  for(;;){
    SetMotorsReadSensors(&s);
    delay(20);
  }
}
