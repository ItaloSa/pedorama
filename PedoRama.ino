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
float stateValue[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

void setup() {
  Serial.begin(9600);
  Serial.println(">> Setup");
  Blynk.begin(auth, ssid, pass);
  homeScreen();
  accelSetup();
  calibrate();
  Alarm.timerOnce(1, pedometerLoop);
  Serial.println(">> End of setup");
  Blynk.virtualWrite(V1, "done!");
}

void homeScreen() {
  Blynk.virtualWrite(V0, "PedoRama");
  Blynk.virtualWrite(V1, "wait...");
  programStatus();
}

void accelSetup() {
  while(!mpu.beginSoftwareI2C(SCL_PIN,SDA_PIN,MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
}

void programStatus() {
  Blynk.notify("PedoRama iniciado!");
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
  stateValue[4] = pinData;
}

BLYNK_WRITE(V4) {
  int pinData = param.asInt();
  Serial.print("Weight: ");
  Serial.println(pinData);
  stateValue[5] = pinData;
}

void loop() {
  led.Update();
  Blynk.run();
  Alarm.delay(100);
}
/*END OF SETUP*/

/*BLYNK FUNCTIONS*/
void changeMode(int pinData) {    
  if (pinData == 1) {
    Serial.println("Pressed Modo:");
    state++;
    if (state > 5) {
      state = 0;
    }
    Serial.println(state);
    changeView(); 
  }
}

void changeView() {
  char statesName[6][16] = {"Steps", "Distance (m)", "Calories (Kcal)", "Speed (m/s)", "Height (cm)", "Weight (Kg)"};
  Blynk.virtualWrite(V0, statesName[state]);
  showData(state);
}

void showData(int thisState) {
  if (state == thisState) {
    Blynk.virtualWrite(V1, stateValue[state]);  
  }
}

/*END OF BLYNK FUNCTIONS*/

/*PEDOMETER FUNCTIONS*/
float threshhold=50.0;

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

  int loopSteps = 0;

  for (int i=0;i<50;i++) {
    Vector normAccel = mpu.readNormalizeAccel();
    xaccl[i]=float(normAccel.XAxis);
    
    yaccl[i]=float(normAccel.YAxis);
    
    zaccl[i]=float(normAccel.ZAxis);
    

    // calculo de min e max de cada eixo
    totvect[i] = sqrt(((xaccl[i]-xavg) * (xaccl[i]-xavg)) + ((yaccl[i] - yavg) * (yaccl[i] - yavg)) + ((zval[i] - zavg)* (zval[i] - zavg)));

    // calculo do comprimento do passo
    totave[i] = (totvect[i] + totvect[i-1]) / 2 ;

    Serial.println(totave[i]);

    //cal steps 
    if (totave[i]>threshhold && flag==0) {
      //steps=steps+1;
      loopSteps += 1;
      flag=1;
    }

    if (totave[i] <threshhold  && flag==1) {
      flag=0;
    }

  }

  steps += loopSteps;
  Serial.println('\n');
  Serial.print("steps=");
  Serial.println(steps);

  float userStepWidth = stepWidth(loopSteps, stateValue[4]); // calcula a largura do passo
  float nowDistance = calcNowDistance(loopSteps, userStepWidth);
  float speed = calcSpeed(nowDistance);
  float calories = calcCalories(speed, stateValue[5]);

  attInstantData(nowDistance, speed, calories);

  Alarm.timerOnce(2, pedometerLoop);

}

void attInstantData(float distance, float speed, float calories) {
  stateValue[0] = steps;
  showData(0);

  stateValue[1] += distance;
  showData(1);

  stateValue[3] = speed;
  showData(3);

  stateValue[2] += calories;
  showData(2);

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

// userHeight - m
float stepWidth(float numStep, int userIntHeight) {
  float userHeight = userIntHeight / 100;
  float userStepWidth = 0;
  if(numStep == 0 || numStep > 2) {
    userStepWidth = userHeight / 5;
  }
  if(numStep == 2 || numStep < 3) {
    userStepWidth = userHeight / 4;
  }
  if(numStep == 3 || numStep < 4) {
    userStepWidth = userHeight / 3;
  }
  if(numStep == 4 || numStep < 5) {
    userStepWidth = userHeight/2;
  }
  if(numStep == 6 || numStep < 8) {
    userStepWidth = userHeight;
  }
  if(numStep >= 8) {
    userStepWidth = userHeight * 1.2;
  }
  return userStepWidth; // m
}

float calcNowDistance(int numStep,float userStepWidth){
  float distance = numStep * userStepWidth;
  return distance;
}

float calcSpeed(float distance){
  float speed = distance / 2;
  return speed;
}

float calcCalories(float speed, float userWeight) {
  if (speed == 0) {
    return 1 * userWeight / 1800;
  } else {
    return speed * userWeight / 400;
  }
}

/*END OF PEDOMETER FUNCTIONS*/
