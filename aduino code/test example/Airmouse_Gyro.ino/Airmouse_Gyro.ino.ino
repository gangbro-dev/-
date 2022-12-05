#include <MPU6050.h>
#include<Wire.h>

const int MPU = 0x68; //MPU6050 I2C주소
int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ, Vx, Vy, Vz, dt;
int AcX_calibrate = 0, AcY_calibrate = 0, AcZ_calibrate = 0,
    GyX_calibrate = 0, GyY_calibrate = 0, GyZ_calibrate = 0;
String Command;
unsigned long timeCurr, timePrev;
float a = 1.1123;

void get6050();
MPU6050 mpu;


void calibrate() {
  AcX_calibrate = 0; AcY_calibrate = 0; AcZ_calibrate = 0;
  GyX_calibrate = 0; GyY_calibrate = 0; GyZ_calibrate = 0;
  int calibrate_rate = 5;
  for (int i = 0; i < calibrate_rate; i++) {
    get6050();
    AcX_calibrate = AcX_calibrate + AcX / calibrate_rate;
    AcY_calibrate = AcY_calibrate + AcY / calibrate_rate;
    AcZ_calibrate = AcZ_calibrate + AcZ / calibrate_rate;
    GyX_calibrate = GyX_calibrate + GyX / calibrate_rate;
    GyY_calibrate = GyY_calibrate + GyY / calibrate_rate;
    GyZ_calibrate = GyZ_calibrate + GyZ / calibrate_rate;
  }
}
void check_calibrate() {
  Serial.print(AcX_calibrate);
  Serial.print(",");
  Serial.print(AcY_calibrate);
  Serial.print(",");
  Serial.print(AcZ_calibrate);
  Serial.print(",");
  Serial.print(GyX_calibrate);
  Serial.print(",");
  Serial.print(GyY_calibrate);
  Serial.print(",");
  Serial.print(GyZ_calibrate);
  Serial.println();
}

void setup()
{
  Vx = 0; Vy = 0; Vz = 0;
  //  mpu.initialize();
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);//MPU6050 을 동작 대기 모드로 변경
  Wire.endTransmission(true);
  //  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  Command = "";
  Serial.begin(115200);
  get6050();
  calibrate();
  //  check_calibrate();
  delay(500);
  timePrev = millis();
}

void loop()
{
  //  int i = 0;
  get6050();//센서값 갱신
  timeCurr = millis();
  dt = timeCurr - timePrev;
  Vx = Vx + (((AcX - AcX_calibrate) * dt) / 1000);
  Vy = Vy + (((AcY - AcY_calibrate) * dt) / 1000);
  Vz = Vz + (((AcZ - AcZ_calibrate) * dt) / 1000);
  Serial.print(Vx);
  Serial.print(",");
  Serial.print(Vy);
  Serial.print(",");
  Serial.print(Vz);
  Serial.println();
  if (Command == "set") {
    Vx = 0; Vy = 0; Vz = 0;
  }
  if (Command == "geta") {
    Serial.print(AcX - AcX_calibrate);
    Serial.print(",");
    Serial.print(AcY - AcY_calibrate);
    Serial.print(",");
    Serial.print(AcZ - AcZ_calibrate);
    Serial.print(",");
    Serial.print(GyX - GyX_calibrate);
    Serial.print(',');
    Serial.print(GyY - GyY_calibrate);
    Serial.print(',');
    Serial.print(GyZ - GyZ_calibrate);
    Serial.println();
  }
  else if (Command == "check calibrate") {
    check_calibrate();
  } else if (Command == "stop") {
    Serial.end();
    while (1);
  }
  Command = Serial.readStringUntil('\n');
  timePrev = timeCurr;
}


void get6050() {
  Wire.beginTransmission(MPU);//MPU6050 호출
  Wire.write(0x3B);//AcX 레지스터 위치 요청
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); //14byte의 데이터를 요청
  AcX = Wire.read() << 8 | Wire.read(); //두개의 나뉘어진 바이트를 하나로 이어붙입니다.
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}
