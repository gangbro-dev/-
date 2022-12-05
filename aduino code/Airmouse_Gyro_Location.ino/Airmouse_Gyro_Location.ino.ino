#include <Wire.h>
#include <math.h>
#include <Mouse.h>

#define PI 3.1416
#define RADIAN PI/180
#define sw 12

const int MPU = 0x68; //MPU6050 I2C주소
int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

const int AcX_calibrate = 30, AcY_calibrate = -83, AcZ_calibrate = 87,
          GyX_calibrate = -43, GyY_calibrate = 30, GyZ_calibrate = -9;
int m1x = 0, m2x = 0, m3x = 0,
    m1y = 0, m2y = 0, m3y = 0,
    m1z = 0, m2z = 0, m3z = 0;
String Command;
long timeCurr, timePrev, dt,
     zero_accel_x_start, zero_accel_y_start, zero_accel_z_start;
int zero_accel_x = 0, zero_accel_y = 0, zero_accel_z = 0, blender_flag = 0;
float GAcX, GAcY, GAcZ, Vx = 0, Vy = 0, Vz = 0, LoX = 0, LoY = 0, LoZ = 0,
                        acc_pitch, acc_roll, acc_yaw, Gy_pitch, Gy_roll, Gy_yaw, angle_pitch, angle_roll, angle_yaw,
                        sin_pitch, cos_pitch, sin_roll, cos_roll, sin_yaw, cos_yaw;
float Prev_GAcX1 = 0, Prev_GAcY1 = 0, Prev_GAcZ1 = 0,
      Prev_GAcX2 = 0, Prev_GAcY2 = 0, Prev_GAcZ2 = 0;

void get6050();       // raw data 센서값을 받아옴
void init_MPU6050();  // MPU6050 설정
void calibrate();     // calibration 값을 raw data에 반영
void calculate_accel_Rotation(); //
void cut_vel();       // 0.2초 이상 일정 가속도값 이하일 경우 속도 0으로 초기화(drift 제거)
void threshold_A();   // calibration 된 데이터에서 가속도 입력값이 작을 때 노이즈값 제거
void threshold_G();   // calibration 된 데이터에서 각속도 입력값이 작을 때 노이즈값 제거
void window_F();      // 가속도센서 raw data window filter
void calculate_vel(); // 가속도 -> 속도 적분
void calculate_loc(); // 속도 -> 거리 적분
void calculate_rot(); // 각속도 -> 각도 적분
void rotation_accel_angle(); // 이동시 계산하는 좌표축을 지구좌표계로 변경
void canceling_Gravity(); // 회전된 센서에서 감지한 중력가속도 제거

void setup()
{
  init_MPU6050();
  Command = "";
  pinMode(sw, INPUT_PULLUP);
  Serial.begin(115200);
  delay(500);
  timePrev = millis();
}

void loop()
{
  get6050();//센서값 갱신
  calibrate();
  //threshold_A();
  //window_F();
  timeCurr = millis();
  dt = timeCurr - timePrev;

  calculate_accel_Rotation();

  calculate_rot();
  
  canceling_Gravity();
  
  threshold_G();

  calculate_vel();

  cut_vel();

  calculate_loc();

  if (Command == "getl") {
    Command = "";
    Serial.print(LoX, 6);
    Serial.print(",");
    Serial.print(LoY, 6);
    Serial.print(",");
    Serial.print(LoZ, 6);
    Serial.println();
  } else if (Command == "geta") {
    Command = "";
    Serial.print(AcX);
    Serial.print(",");
    Serial.print(AcY);
    Serial.print(",");
    Serial.print(AcZ);
    Serial.print(",");
    Serial.print(angle_pitch / 57.29578, 4);
    Serial.print(',');
    Serial.print(angle_roll / 57.29578, 4);
    Serial.print(',');
    Serial.print(angle_yaw / 57.29578, 4);
    Serial.println();
  } else if (Command == "stop") {
    Command = "";
    Vx = 0; Vy = 0; Vz = 0;
    LoX = 0; LoY = 0; LoZ = 0;
    blender_flag = 0;
    //Serial.end();
    //while(1);
  } else if (Command == "set") {
    Command = "";
    Vx = 0; Vy = 0; Vz = 0;
    LoX = 0; LoY = 0; LoZ = 0;
    blender_flag = 1;
    //timeCurr = millis();
  } else {
    Command = "";
  }
  
  //if (blender_flag == 0)
    //Mouse.move((char)(17 * Vy), (char)(-17 * Vz), 0);
    
  if (digitalRead(sw) == LOW)
    Mouse.press(MOUSE_LEFT);
  else
    Mouse.release(MOUSE_LEFT);
  if (Serial.available() > 0) {
    Command = Serial.readStringUntil('\n');
  }

  timePrev = timeCurr;
  ////////////////////////////////////////////출력값 체크 칸 //////////////////////////////////
  //Serial.print(" || A(x,y,z) = (");
  //Serial.print(AcX); Serial.print(",");
  //Serial.print(AcY); Serial.print(",");
  //Serial.print(AcZ); Serial.print(")");
  //Serial.print(" || GA(x,y,z) = (");
  //Serial.print(GAcX,4); Serial.print(",");
  //Serial.print(GAcY,4); Serial.print(",");
  //Serial.print(GAcZ,4); Serial.print(")");
  //Serial.print(" || Angle(p, r, y) = (");
  //Serial.print(angle_pitch); Serial.print(",");
  //Serial.print(angle_roll); Serial.print(",");
  //Serial.print(angle_yaw);
  //Serial.print(")");
  Serial.print(" || V(x,y,z) = ("); ;
  Serial.print(Vx, 6); Serial.print(",");
  Serial.print(Vy, 6); Serial.print(",");
  Serial.print(Vz, 4);Serial.print(","); Serial.print(")");
  Serial.print(" || L(x,y,z) = (");
  Serial.print(LoX, 4); Serial.print(",");
  Serial.print(LoY, 4); Serial.print(",");
  Serial.print(LoZ, 4);
  //Serial.print(" || dt= ");
  //Serial.print(dt);

  Serial.println(")");
  ///////////////////////////////////////////////////////////////////////////////////////////
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

void init_MPU6050() {
  //MPU6050 Initalizing & Reset
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);//MPU6050 을 동작 대기 모드로 변경
  Wire.endTransmission(true);

  //MPU6050 Clock Type
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);//PWR_MGMT_1 register
  Wire.write(0x03);//Selection Clock 'PLL with Z axis gyroscope reference'
  Wire.endTransmission(true);

  //MPU6050 Gyroscope Configuration Setting
  Wire.beginTransmission(MPU);
  Wire.write(0x1B);   //Gyroscope Configuration register
  //Wire.write(0x00); //FS_SEL=0, Full Scale Range = +/- 250[degree/sec]
  //Wire.write(0x08); //FS_SEL=1, Full Scale Range = +/- 500[degree/sec]
  //Wire.write(0x10);   //FS_SEL=2, Full Scale Range = +/- 1000[degree/sec]
  Wire.write(0x18); //FS_SEL=3, Full Scale Range = +/- 2000[degree/sec]
  Wire.endTransmission(true);

  //MPU6050 Accelerometer Configuration Setting
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);   //Accelerometer Configuration register
  //Wire.write(0x00); //AFS_SEL=0, Full Scale Range = +/- 2[g]
  //Wire.write(0x08); //AFS_SEL=1, Full Scale Range = +/- 4[g]
  Wire.write(0x10);   //AFS_SEL=2, Full Scale Range = +/- 8[g]
  //Wire.write(0x18); //AFS_SEL=3, Full Scale Range = +/- 10[g]
  Wire.endTransmission(true);

  //MPU6050 DLPF(Digital Low Pass Filter)
  Wire.beginTransmission(MPU);
  Wire.write(0x1A);  //DLPF_CFG register
  //Wire.write(0x00);//Accel BW 260Hz,  Delay 0ms     / Gyro BW 256Hz, Delay 0.98ms, Fs 8KHz
  //Wire.write(0x01);//Accel BW 184Hz,  Delay 2ms     / Gyro BW 188Hz, Delay 1.9ms,  Fs 8KHz
  Wire.write(0x02);//Accel BW 94Hz,   Delay 3ms     / Gyro BW 98Hz,  Delay 2.8ms,  Fs 8KHz
  //Wire.write(0x03);//Accel BW 44Hz,   Delay 4.9ms   / Gyro BW 42Hz,  Delay 4.8ms,  Fs 8KHz
  //Wire.write(0x04);//Accel BW 21Hz,   Delay 8.5ms   / Gyro BW 20Hz,  Delay 8.3ms,  Fs 8KHz
  //Wire.write(0x05);//Accel BW 10Hz,   Delay 13.8ms  / Gyro BW 10Hz,  Delay 13.4ms, Fs 8KHz
  //Wire.write(0x06);//Accel BW 5Hz,    Delay 19ms    / Gyro BW 5Hz,   Delay 18.6ms, Fs 8KHz
  Wire.endTransmission(true);
}

void calibrate() {
  AcX -= AcX_calibrate;
  AcY -= AcY_calibrate;
  AcZ -= AcZ_calibrate;
  GyX -= GyX_calibrate;
  GyY -= GyY_calibrate;
  GyZ -= GyZ_calibrate;
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

void cut_vel() {

  unsigned char cut_time = 50;
  //x-axis velocity cutting
  if (GAcX == 0 & zero_accel_x == 0) {
    zero_accel_x = 1; zero_accel_x_start = millis();
  } else if (GAcX == 0 & zero_accel_x == 1) {}
  else
    zero_accel_x = 0;

  if (zero_accel_x == 1 & (millis() - zero_accel_x_start) > cut_time)
    Vx = 0;

  //y-axis velocity cutting
  if (GAcY == 0 & zero_accel_y == 0) {
    zero_accel_y = 1;
    zero_accel_y_start = millis();
  }  else if (GAcY == 0 & zero_accel_y == 1) {
  }  else {
    zero_accel_y = 0;
  }

  if (zero_accel_y == 1 & (millis() - zero_accel_y_start) > cut_time) {
    Vy = 0;
  }
  //if (Prev_GAcY2 == Prev_GAcY1 & Prev_GAcY1 == GAcY)
  //Vy = 0;
  //z-axis velocity cutting
  if (GAcZ == 0 & zero_accel_z == 0) {
    zero_accel_z = 1;
    zero_accel_z_start = millis();
  }  else if (GAcZ == 0 & zero_accel_z == 1) {
  }  else {
    zero_accel_z = 0;
  }

  if (zero_accel_z == 1 & (millis() - zero_accel_z_start) > cut_time) {
    Vz = 0;
  }
  Prev_GAcX2 = Prev_GAcX1; Prev_GAcY2 = Prev_GAcY1; Prev_GAcZ2 = Prev_GAcZ1;
  Prev_GAcX1 = GAcX, Prev_GAcY1 = GAcY, Prev_GAcZ1 = GAcZ;
}

void threshold_A() {
  int threshold = 80;
  if (abs(AcX) < threshold) {
    AcX = 0;
  }
  if (abs(AcY) < threshold) {
    AcY = 0;
  }
  if (abs(AcZ) < threshold) {
    AcZ = 0;
  }
}
void threshold_G() {
  float threshold = 0.05;
  float threshold_z = 0.05;
  if (abs(GAcX) < threshold) {
    GAcX = 0;
  }
  if (abs(GAcY) < threshold) {
    GAcY = 0;
  }
  if (abs(GAcZ) < threshold_z) {
    GAcZ = 0;
  }
}
void window_F() {
  m3x = m2x; m2x = m1x; m1x = AcX; AcX = (m1x / 3) + (m2x / 3) + (m3x / 3);
  m3y = m2y; m2y = m1y; m1y = AcY; AcY = (m1y / 3) + (m2y / 3) + (m3y / 3);
  m3z = m2z; m2z = m1z; m1z = AcZ; AcZ = (m1z / 3) + (m2z / 3) + (m3z / 3);

}

void calculate_vel() {
  Vx += (((GAcX * 9.81) * dt) / 1000);
  Vy += (((GAcY * 9.81) * dt) / 1000);
  Vz += (((GAcZ * 9.81) * dt) / 1000);
}

void calculate_loc() {
  LoX += (((Vx) * dt) / 1000);
  LoY += (((Vy) * dt) / 1000);
  LoZ += (((Vz) * dt) / 1000);
}

void calculate_accel_Rotation() {
  // Convert accelerometer to gravity value
  GAcX = (float) AcX / 4096.0;
  GAcY = (float) AcY / 4096.0;
  GAcZ = (float) AcZ / 4096.0;

  // Calculate Pitch, Roll & Yaw from Accelerometer value
  acc_pitch = atan ((GAcY) / sqrt(GAcX * GAcX + GAcZ * GAcZ)) * 57.29577951; // 180 / PI = 57.29577951
  acc_roll = - atan ((GAcX) / sqrt(GAcY * GAcY + GAcZ * GAcZ)) * 57.29577951;
  //acc_yaw = atan ((GAcZ - (float)AcZ_calibrate/4096.0) / sqrt(GAcX * GAcX + GAcZ * GAcZ)) * 57.29577951;
  acc_yaw = atan (sqrt((GAcX * GAcX) + (GAcZ * GAcZ)) / (GAcZ / 4096.0)) * 57.29577951;

}

void calculate_rot() {
  float alpha = 0.996;

  Gy_pitch += (float)GyX / 16384 * dt;
  Gy_roll += (float)GyY / 16384 * dt;
  Gy_yaw += (float)GyZ / 16384 * dt;
  angle_pitch = (alpha * (((float)GyX / 16384 * dt) + angle_pitch)) + ((1 - alpha) * acc_pitch);
  angle_roll = (alpha * (((float)GyY / 16384 * dt) + angle_roll)) + ((1 - alpha) * acc_roll);
  angle_yaw = Gy_yaw;
}

void rotation_accel_angle() {
  sin_pitch = sin((angle_pitch * PI / 180)); cos_pitch = cos(angle_pitch * PI / 180);
  sin_roll = sin(angle_roll * PI / 180); cos_roll = cos(angle_roll * PI / 180);
  sin_yaw = sin(angle_yaw * PI / 180); cos_yaw = cos(angle_yaw * PI / 180);
  float GAcX_temp = GAcX, GAcY_temp = GAcY, GAcZ_temp = GAcZ;
  GAcX = ((cos_pitch * cos_yaw) * GAcX_temp) + (((sin_roll * sin_pitch * cos_yaw) - (cos_roll * sin_yaw)) * GAcY_temp) +
         (((cos_roll * sin_pitch * cos_yaw) + (sin_roll * sin_yaw)) * GAcZ_temp);
  GAcY = ((cos_pitch * sin_yaw) * GAcX_temp) + (((sin_roll * sin_pitch * sin_yaw) + (cos_roll * cos_yaw)) * GAcY_temp) +
         (((cos_roll * sin_pitch * sin_yaw) - (sin_roll * cos_yaw)) * GAcZ_temp);
  GAcZ = ((-sin_pitch) * GAcX_temp) + ((sin_roll * cos_pitch) * GAcY_temp) + ((cos_roll * cos_pitch) * GAcZ_temp);
}

void canceling_Gravity() {
  GAcX += sin(angle_roll * RADIAN);
  GAcY -= sin(angle_pitch * RADIAN);
  GAcZ -= sqrt(abs(1 - sq(sin(angle_roll * RADIAN)) - sq(sin(angle_pitch * RADIAN))));
}
