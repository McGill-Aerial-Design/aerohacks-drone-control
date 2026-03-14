#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


#include <WiFi.h>
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
WiFiServer tcpServer(8080);
WiFiClient client;

Adafruit_MPU6050 mpu;

byte pinA = 5;
byte pinB = 6;
byte pinC = 3;
byte pinD = 4;

byte mode = 0;
byte modeZ = 0;

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

float gyroOffsetX = 0;
float gyroOffsetY = 0;

float vz = 0;
float vx = 0;
float vy = 0;
float x = 0;
float y = 0;
float z = 0;
float gyroX = 0;
float gyroY = 0;

int thrustA = 0;
int thrustB = 0;
int thrustC = 0;
int thrustD = 0;

unsigned long lastTime = 0;
 
void setup() {
  Serial.begin(115200);
  pinMode(7, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(7, HIGH); // LED blue
  digitalWrite(9, LOW);
	/*ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);


	A.setPeriodHertz(50);
	A.attach(pinA, 2, 19000);
	B.setPeriodHertz(50);
	B.attach(pinB, 2, 19000);
	C.setPeriodHertz(50);
	C.attach(pinC, 2, 19000);
	D.setPeriodHertz(50);
	D.attach(pinD, 2, 19000);*/

  Wire.begin(11,10);
  if (!mpu.begin(0x68)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {delay(10);}
  }
  //mpu.calcOffsets(true, true);
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  
  sensors_event_t a, g, temp;

  unsigned int numCalibReadings = 2000;

  Serial.println("Callibrating, please wait");

  for (unsigned int i=0; i<numCalibReadings; i++) {
    mpu.getEvent(&a, &g, &temp);
    gyroOffsetX += g.gyro.x;
    gyroOffsetY += g.gyro.y;
    delay(2);
  }
  gyroOffsetX /= numCalibReadings;
  gyroOffsetY /= numCalibReadings;

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("AeroHacks Drone 1", "skibidi123");
  tcpServer.begin();

  Serial.println("ready");
  digitalWrite(7, LOW); // LED green
  digitalWrite(9, HIGH);

  lastTime = millis();
}








void loop() {
  unsigned long newTime = millis();
  unsigned int dt = newTime - lastTime;
  lastTime = newTime;


  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float gyroVX = g.gyro.x - gyroOffsetX;
  float gyroVY = g.gyro.y - gyroOffsetY;
  gyroX -= gyroVX * dt;
  gyroY -= gyroVY * dt;
  float rawAccZ = a.acceleration.z;
  float rawAccX = a.acceleration.x;
  float rawAccY = a.acceleration.y;
  float accZ = rawAccZ * cos(gyroX * PI/180) * cos(gyroY * PI/180) - rawAccX * sin(gyroX * PI/180) - rawAccY * sin(gyroY * PI/180);
  float accX = rawAccX * cos(gyroX * PI/180) + rawAccZ * sin(gyroX * PI/180);
  float accY = rawAccY * cos(gyroY * PI/180) + rawAccZ * sin(gyroY * PI/180);



  if (!client) {client = tcpServer.available();}
  else if (!client.connected()) {
    client.stop();
    mode = 0;
  }

  if (client.available()) {
    String instruct = client.readStringUntil('\n');

    if (instruct == "ping") {
      client.print("ping");
    }

    else if (instruct == "angX") {client.print(String(gyroX));}
    else if (instruct == "angY") {client.print(String(gyroY));}
    else if (instruct == "gyroX") {client.print(String(gyroVX));}
    else if (instruct == "gyroY") {client.print(String(gyroVY));}
    else if (instruct == "gMode") {client.print(String(mode));}
    else if (instruct == "gModeZ") {client.print(String(modeZ));}
    
    else if (instruct.startsWith("mode")) {
      instruct.remove(0, 4);
      mode = instruct.toInt();
      Serial.print("New Mode: ");
      Serial.println(mode);
    }
    
    else if (instruct.startsWith("zmode")) {
      instruct.remove(0, 4);
      modeZ = instruct.toInt();
    }
    
    else if (instruct.startsWith("az")) {
      instruct.remove(0, 2);
      targetAZ = instruct.toInt();
    }
    
    else if (instruct.startsWith("vz")) {
      instruct.remove(0, 2);
      targetVZ = instruct.toInt();
    }
    
    else if (instruct.startsWith("gvx")) {
      instruct.remove(0, 3);
      targetGyroVX = instruct.toInt();
    }
    
    else if (instruct.startsWith("gvy")) {
      instruct.remove(0, 3);
      targetGyroVY = instruct.toInt();
    }
    
    else if (instruct.startsWith("gx")) {
      instruct.remove(0, 2);
      targetGyroX = instruct.toInt();
    }
    
    else if (instruct.startsWith("gy")) {
      instruct.remove(0, 2);
      targetGyroY = instruct.toInt();
    }

    else if (instruct == "manT") {
      thrustA = client.readStringUntil(',').toInt();
      thrustB = client.readStringUntil(',').toInt();
      thrustC = client.readStringUntil(',').toInt();
      thrustD = client.readStringUntil('\n').toInt();
    }




    client.print("\n");
  }

  if (mode == 1) {modeZ = 1;}
  if (mode == 0) {modeZ = 0;}


  //Serial.println(gyroX);

  if (modeZ >= 4) {
    if (z < targetZ) {targetVZ += 1 * dt;}
    else if (z > targetZ) {targetVZ -= 1 * dt;}
  }

  if (modeZ >= 3) {
    if (targetVZ > MAX_SPEED) {targetVZ = MAX_SPEED;}
    if (targetVZ < -MAX_SPEED) {targetVZ = -MAX_SPEED;}
  }

  if (modeZ >= 2) {
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
  }











  if (mode >= 2) {
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
  }

  if (mode >= 3) {
    if (gyroX < targetGyroX) {targetGyroVX += 0.1;}
    else if (gyroX > targetGyroX) {targetGyroVX -= 0.1;}

    if (targetGyroVX > MAX_ANGULAR_VELOCITY) {targetGyroVX = MAX_ANGULAR_VELOCITY;}
    else if (targetGyroVX < -MAX_ANGULAR_VELOCITY) {targetGyroVX = -MAX_ANGULAR_VELOCITY;}
  }

  if (targetGyroX > MAX_ANGLE) {targetGyroX = MAX_ANGLE;}
  else if (targetGyroX < -MAX_ANGLE) {targetGyroX = -MAX_ANGLE;}


  if (mode >= 2) {
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
  if (thrustA > 250) {thrustA = 250;}
  if (thrustB > 250) {thrustB = 250;}
  if (thrustC > 250) {thrustC = 250;}
  if (thrustD > 250) {thrustD = 250;}

  x += vx * dt;
  y += vy * dt;
  z += vz * dt;

  vx += accX * dt;
  vy += accY * dt;
  vz += accZ * dt;

  if (mode == 0) {
    thrustA = 0;
    thrustB = 0;
    thrustC = 0;
    thrustD = 0;
  }

  //A.write(thrustA);
  //B.write(thrustB);
  //C.write(thrustC);
  //D.write(thrustD);
  analogWrite(pinA, thrustA);
  analogWrite(pinB, thrustB);
  analogWrite(pinC, thrustC);
  analogWrite(pinD, thrustD);
}
