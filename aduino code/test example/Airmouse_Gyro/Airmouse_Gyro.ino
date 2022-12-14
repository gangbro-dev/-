#include <Mouse.h>
#include <Wire.h>
#include <MPU6050.h>


MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;
int vx, vy;

String CommandFromPC;
const int trigPin = 9;
const int echoPin = 10;
float duration, distance;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  if(!mpu.testConnection()) {
    while(1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0)
  {
    CommandFromPC = Serial.readStringUntil('\n');
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    vx = (gx+300)/200;  // "+300" because the x axis of gyroscope give values about -350 while it's not moving. Change this value if you get something different using the TEST code, chacking if there are values far from zero.
    vy = -(gz-100)/200; // same here about "-100"
    
    Mouse.move(vx,vy);
      
    delay(20);
  }
}
