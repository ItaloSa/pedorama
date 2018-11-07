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

// accel
MPU6050 mpu;
int SCL_PIN=D6;
int SDA_PIN=D5;

// led
JLed led = JLed(D4);

// program
int state = 6;
int stateValue[5] = {0};

void setup() {
  Serial.begin(9600);
  Serial.println(">> Setup");
  programStatus();
  accelSetup();
  calibrate();
  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(V0, "PedoRama");
  Alarm.timerOnce(1, pedometerLoop);
  Serial.println(">> End of setup");
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
  Serial.println(">> Connected");
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
/*END OF SETUP*/

/*BLYNK FUNCTIONS*/
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

void showData(int thisState) {
  if (state == thisState) {
    Blynk.virtualWrite(V1, stateValue[state]);  
  }
}

/*END OF BLYNK FUNCTIONS*/

/*PEDOMETER FUNCTIONS*/
float threshhold=25.0;

// valores da calibracao 
float xval[50]={0};
float yval[50]={0};
float zval[50]={0};

// medias
float xavg; 
float yavg;
float zavg;
int steps, flag = 0;

void pedometerLoop() {
  Serial.println(">> Loop");
  float totvect[50]={0}; // vetor de aceleracao
  float totave[50]={0}; // vetor de medias

  // aceleracoes no espaco de 50 interacoes
  float xaccl[50]={0};
  float yaccl[50]={0};
  float zaccl[50]={0};

  for (int i=0;i<50;i++) {
    Vector normAccel = mpu.readNormalizeAccel();
    xaccl[i]=float(normAccel.XAxis);
    delay(1);
    yaccl[i]=float(normAccel.YAxis);
    delay(1);
    zaccl[i]=float(normAccel.ZAxis);
    delay(1);

    // calculo de min e max de cada eixo
    totvect[i] = sqrt(((xaccl[i]-xavg) * (xaccl[i]-xavg)) + ((yaccl[i] - yavg) * (yaccl[i] - yavg)) + ((zval[i] - zavg)* (zval[i] - zavg)));

    // calculo do comprimento do passo
    totave[i] = (totvect[i] + totvect[i-1]) / 2 ;

    Serial.println(totave[i]);
    delay(200);

    //cal steps 
    if (totave[i]>threshhold && flag==0) {
      steps=steps+1;
      flag=1;
    } else if (totave[i] > threshhold && flag==1) {
      //do nothing 
    }

    if (totave[i] <threshhold  && flag==1) {
      flag=0;
    }

    Serial.println('\n');
    Serial.print("steps=");
    Serial.println(steps);
  }

  if (steps > stateValue[0]) {
    stateValue[0] = steps;
    showData(0);
  }
  Alarm.timerOnce(1, pedometerLoop);

}

// Construir o filtro
void calibrate() {
  Serial.println(">> Calibrnado....");
  float sum=0;
  float sum1=0;
  float sum2=0;
  Vector normAccel = mpu.readNormalizeAccel();

  // soma os valores do pino e seta no array 
  for (int i=0;i<50;i++) {
    xval[i] = float(normAccel.XAxis);
    sum=xval[i]+sum;
  }
  xavg=sum/50.0;
  Serial.println(xavg);
  delay(100);


  for (int j=0;j<50;j++) {
    yval[j]=float(normAccel.YAxis);
    sum1=yval[j]+sum1;
  }
  yavg=sum1/50.0;
  Serial.println(yavg);
  delay(100);

  for (int k=0;k<50;k++) {
    zval[k]=float(normAccel.ZAxis);
    sum2=zval[k]+sum2;
  }
  zavg=sum2/50.0;
  Serial.println(zavg);
  delay(100);

  Serial.println(">> Calibrado!");
}

/*END OF PEDOMETER FUNCTIONS*/
