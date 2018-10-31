/*IMPORTS*/
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <string>
#include <jled.h>

/*CONSTS*/
#define BLYNK_PRINT Serial

/*SETUP*/
char auth[] = "faa3a3fc41bd4c2f90dfd9b7ffbfd3b8";
char ssid[] = "jualabs-ap";
char pass[] = "jualabsufrpe";

bool isFirstConnect = true;
JLed led = JLed(D4);
int state = 6;

void setup() {
  Serial.begin(9600);
  led.On().Update();
  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(V0, "PedoRama");  
  programStatus();
}

void programStatus() {
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

void loop() {
  Blynk.run();
  led.Update();
}

/*FUNCTIONS*/
void changeMode(pinData) {    
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
