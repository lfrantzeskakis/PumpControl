#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#define THERMISTORNOMINAL 10000      
#define TEMPERATURENOMINAL 25   
#define BCOEFFICIENT 3950 
#define SERIESRESISTOR 10000  
float R1 = 10000;

LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7);
void redAlarm(int);
void greenAlarm(void);
void alarm(void);
void stopWorking(void);
void startWorking(void);
byte waitForwater(void);
void mPrint(String,String);

float tempVal1=0;
float tempVal2=0;
int setTry=0;
volatile byte exittemp=1;
static int Osmosis=12;
static int Pumps=11;
static int TempVcc=7;
static int AlarmButton=2; //interrupt button
static int FloterSwitch=10;
static int LedRed=5;
static int LedGreen=6;
static int Temp1=0;
static int Temp2=1;
static int FloterVcc=8;
int period = 5000;
byte show=1;

unsigned long time_now = 0;


void mPrint(String messageT,String messageB){
  lcd.clear();
  lcd.home();
  lcd.print(messageT);
  lcd.setCursor(0,1);
  lcd.print(messageB);
}

void greenAlarm(){
  int i;
  for(i=0;i<254;i++){
  analogWrite(LedGreen, i); 
  delay(1);
  }
  for(i=255;i>0;i--){
  analogWrite(LedGreen, i); 
  delay(1);
  } 
}
void redAlarm(int a){
  int i;
  for(i=0;i<254;i++){
  analogWrite(LedRed, i); 
  delay(a);
  }
  for(i=255;i>0;i--){
  analogWrite(LedRed, i); 
  delay(a);
  }
}
void exitState(){
  exittemp=0;
  setTry=0;
}
void startWorking(){
 digitalWrite(Pumps,LOW);
 mPrint("<<<<Starting>>>>","Pump");
 redAlarm(5);
 delay(5000);
 redAlarm(5);
 mPrint("<<<<Starting>>>>","Osmosis");
 digitalWrite(Osmosis,LOW);
 delay(2000);
}
void stopWorking(){
 digitalWrite(Osmosis,HIGH);
 mPrint("<<<<Closing>>>>>","Osmosis");
 redAlarm(5);
 delay(5000);
 redAlarm(5);
 mPrint("<<<<Closing>>>>>","Pumps");
 digitalWrite(Pumps,HIGH);
 delay(2000);
}
void alarm(){
 stopWorking();
 while(exittemp){
    mPrint("HTemp Error ","Press Button"); 
    Serial.println("ALARM !!!!!");
    redAlarm(1);
 }
 startWorking();
 exittemp=1; 
}
byte waitForwater(){
  //byte water= digitalRead(FloterSwitch);
  digitalWrite(FloterVcc,HIGH);
  int AR2 = analogRead(2);
  digitalWrite(FloterVcc,LOW);
  byte water;
  if(AR2<900){water=0;mPrint("Working Normal","Floter UP "+String(AR2));}
  else{water=1;mPrint("No Water!!","Floter Down"+String(AR2));}    
return water;
}
void SystemClose(){
 //Serial.println("Close system");
 mPrint("Close Systems","Error: No Water");
 stopWorking();
 while(waitForwater()){
  redAlarm(5);
 }
 startWorking();
}

float readCelcius(int sens){
  digitalWrite(TempVcc,HIGH);
  int Vo = analogRead(sens);
  float R2 = R1 * (1023.0 / (float)Vo - 1.0);
  float steinhart;
  steinhart = R2 / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;
  digitalWrite(TempVcc,LOW);
  return steinhart;  
}
void tempRead(){
  tempVal1=readCelcius(0);
  tempVal2=readCelcius(1);
  if(tempVal1>45.0 || tempVal2>45.0){ 
    stopWorking();
    setTry++;
     while(tempVal1>35.0 || tempVal2>35.0 ){
     mPrint("High Temp","ALARM!!!");
     tempVal1=readCelcius(Temp1);
     redAlarm(4);
     tempVal2=readCelcius(Temp2);
     mPrint("Wait until 35C",String(tempVal1)+"C "+String(tempVal2)+"C");
     redAlarm(4);
     }
    startWorking();
  }
}
void setup() {
  Serial.begin(9600);
  pinMode(Osmosis,OUTPUT);
  pinMode(Pumps,OUTPUT);
  pinMode(TempVcc,OUTPUT);
  pinMode(LedRed,OUTPUT);
  pinMode(LedGreen,OUTPUT);
  pinMode(AlarmButton,INPUT_PULLUP);
  pinMode(FloterVcc,OUTPUT);
  digitalWrite(TempVcc,LOW);
  digitalWrite(Pumps,LOW);
  digitalWrite(Osmosis,LOW);
  attachInterrupt(digitalPinToInterrupt(AlarmButton), exitState , LOW);
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home (); 
  mPrint("Frantzeskakis","Lambros");
  delay(1000);
  mPrint("SparkyMind","PumpControl");
  delay(1000);
}

void loop(){
  greenAlarm();
  tempRead();
  if(waitForwater())SystemClose();
  //Serial.print("ok ");
  if(millis() > time_now + period){
        time_now = millis();
         if(show==1){mPrint("Temperature Alarm",String(setTry));show=0;}
         else{mPrint("Temperature Now ",String(tempVal1)+"C "+String(tempVal2)+"C");show=1;}
    }
delay(50);
}
