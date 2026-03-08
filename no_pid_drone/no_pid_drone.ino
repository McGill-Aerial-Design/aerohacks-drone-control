// Completely untested for now. I just wrote this while I was bored in the 12 hour train ride


#include <ESP32Servo.h>
#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);
 
Servo A;
Servo B;
Servo C;
Servo D;

const float MAX_ANGULAR_VELOCITY = 5;
const float MAX_ANGLE = 20;
const float MAX_SPEED = 1; // arbitrary change later

float targetAZ = 0;
float targetVZ = 0;
float targetZ = 0;
float targetGyroVX = 0;
float targetGyroVY = 0;
float targetGyroX = 0;
float targetGyroY = 0;

float vz = 0;
float vx = 0;
float vy = 0;

float x = 0;
float y = 0;
float z = 0;

int thrustA = 0;
int thrustB = 0;
int thrustC = 0;
int thrustD = 0;

unsigned long lastTime = 0;
 
void setup() {
	//ESP32PWM::allocateTimer(0);
	//ESP32PWM::allocateTimer(1);
	//ESP32PWM::allocateTimer(2);
	//ESP32PWM::allocateTimer(3);
	A.setPeriodHertz(50);
	A.attach(2, 500, 2400);
	B.setPeriodHertz(50);
	B.attach(4, 500, 2400);
	C.setPeriodHertz(50);
	C.attach(12, 500, 2400);
	D.setPeriodHertz(50);
	D.attach(13, 500, 2400);
  lastTime = millis();
}

void loop() {
  unsigned long newTime = millis();
  unsigned int dt = newTime - lastTime;
  lastTime = newTime;


  mpu.update();
  float gyroX = mpu.getAngleX();
  float gyroY = mpu.getAngleY();
  float gyroVX = mpu.getGyroX();
  float gyroVY = mpu.getGyroY();
  float rawAccZ = mpu.getAccZ();
  float rawAccX = mpu.getAccX();
  float rawAccY = mpu.getAccY();
  float accZ = rawAccZ * cos(gyroX * PI/180) * cos(gyroY * PI/180) - rawAccX * sin(gyroX * PI/180) - rawAccY * sin(gyroY * PI/180);
  float accX = rawAccX * cos(gyroX * PI/180) + rawAccZ * sin(gyroX * PI/180);
  float accY = rawAccY * cos(gyroY * PI/180) + rawAccZ * sin(gyroY * PI/180);


  if (z < targetZ) {targetVZ += 1 * dt;}
  else if (z > targetZ) {targetVZ -= 1 * dt;}

  if (targetVZ > MAX_SPEED) {targetVZ = MAX_SPEED;}
  if (targetVZ < -MAX_SPEED) {targetVZ = -MAX_SPEED;}

  if (accZ < targetAZ) {
    thrustA += 1 * dt;
    thrustB += 1 * dt;
    thrustC += 1 * dt;
    thrustD += 1 * dt;
  }
  else if (accZ > targetAZ) {
    thrustA -= 1 * dt;
    thrustB -= 1 * dt;
    thrustC -= 1 * dt;
    thrustD -= 1 * dt;
  }

  if (gyroVX < targetGyroVX) {
    thrustA += 1 * dt;
    thrustB -= 1 * dt;
    thrustC += 1 * dt;
    thrustD -= 1 * dt;
  }
  else if (gyroVX > targetGyroVX) {
    thrustA -= 1 * dt;
    thrustB += 1 * dt;
    thrustC -= 1 * dt;
    thrustD += 1 * dt;
  }

  if (gyroX < targetGyroX) {targetGyroVX += 0.1;}
  else if (gyroX > targetGyroX) {targetGyroVX -= 0.1;}

  if (targetGyroVX > MAX_ANGULAR_VELOCITY) {targetGyroVX = MAX_ANGULAR_VELOCITY;}
  else if (targetGyroVX < -MAX_ANGULAR_VELOCITY) {targetGyroVX = -MAX_ANGULAR_VELOCITY;}

  if (targetGyroX > MAX_ANGLE) {targetGyroX = MAX_ANGLE;}
  else if (targetGyroX < -MAX_ANGLE) {targetGyroX = -MAX_ANGLE;}


  if (gyroVY < targetGyroVY) {
    thrustA += 1 * dt;
    thrustB += 1 * dt;
    thrustC -= 1 * dt;
    thrustD -= 1 * dt;
  }
  else if (gyroVY > targetGyroVY) {
    thrustA -= 1 * dt;
    thrustB -= 1 * dt;
    thrustC += 1 * dt;
    thrustD += 1 * dt;
  }

  if (gyroY < targetGyroY) {targetGyroVY += 0.1;}
  else if (gyroY > targetGyroY) {targetGyroVY -= 0.1;}

  if (targetGyroVY > MAX_ANGULAR_VELOCITY) {targetGyroVY = MAX_ANGULAR_VELOCITY;}
  else if (targetGyroVY < -MAX_ANGULAR_VELOCITY) {targetGyroVY = -MAX_ANGULAR_VELOCITY;}

  if (targetGyroY > MAX_ANGLE) {targetGyroY = MAX_ANGLE;}
  else if (targetGyroY < -MAX_ANGLE) {targetGyroY = -MAX_ANGLE;}


  if (thrustA < 0) {thrustA = 0;}
  if (thrustB < 0) {thrustB = 0;}
  if (thrustC < 0) {thrustC = 0;}
  if (thrustD < 0) {thrustD = 0;}
  if (thrustA > 180) {thrustA = 180;}
  if (thrustB > 180) {thrustB = 180;}
  if (thrustC > 180) {thrustC = 180;}
  if (thrustD > 180) {thrustD = 180;}

  x += vx * dt;
  y += vy * dt;
  z += vz * dt;

  vx += accX * dt;
  vy += accY * dt;
  vz += accZ * dt;

  A.write(thrustA);
  B.write(thrustB);
  C.write(thrustC);
  D.write(thrustD);
}
