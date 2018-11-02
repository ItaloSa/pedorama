/*IMPORTS*/
#include <BlynkSimpleEsp8266.h>
#include <jled.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TimeAlarms.h>

/*CONSTS*/
#define BLYNK_PRINT Serial

/*SETUP*/
// WIFI
char auth[] = "faa3a3fc41bd4c2f90dfd9b7ffbfd3b8";
char ssid[] = "dexbie-ap";
char pass[] = "8hq8xba9";
bool isFirstConnect = true;

// Accel
MPU6050 mpu;
int SCL_PIN=D6;
int SDA_PIN=D5;

// led
JLed led = JLed(D4);

// program
int state = 6;

void setup() {
  Serial.begin(9600);
  programStatus();
  accelSetup();
  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(V0, "PedoRama");
  Alarm.timerRepeat(1, pedometerLoop);
}

void accelSetup() {
  while(!mpu.beginSoftwareI2C(SCL_PIN,SDA_PIN,MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
}

void programStatus() {
  Blynk.notify("PedoRama Working!");
  led.Blink(900, 100).Forever().Update();
}

BLYNK_CONNECTED() {
  if (isFirstConnect) {
    Blynk.syncAll();
    isFirstConnect = false;
  }
}

BLYNK_WRITE(V2) {
  int pinData = param.asInt();
  changeMode(pinData);
}

BLYNK_WRITE(V3) {
  int pinData = param.asInt();
  Serial.print("Height: ");
  Serial.println(pinData);
}

BLYNK_WRITE(V4) {
  int pinData = param.asInt();
  Serial.print("Weight: ");
  Serial.println(pinData);
}

void loop() {
  led.Update();
  Blynk.run();
  Alarm.delay(100);
}

/*FUNCTIONS*/
void changeMode(int pinData) {    
  Serial.print(pinData);
  if (pinData == 1) {
    Serial.println("Pressed Modo");
    state++;
    if (state > 5) {
      state = 0;
    }
    changeView(); 
  }
}

void changeView() {
  char statesName[6][10] = {"Steps", "Distance", "Calories", "Speed", "Height", "Weight"};
  Blynk.virtualWrite(V0, statesName[state]);
}

void pedometerLoop() {
  Vector normAccel = mpu.readNormalizeAccel();
  Serial.print(" Xnorm = ");
  Serial.print(normAccel.XAxis);
  Serial.print(" Ynorm = ");
  Serial.print(normAccel.YAxis);
  Serial.print(" Znorm = ");
  Serial.println(normAccel.ZAxis);
}
