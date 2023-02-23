// Import thư viện
#include <LiquidCrystal.h> // Khai báo thư viện LCD / Kết nối qua SPI (Được hỗ trợ sẵn)
#include "Wire.h"
#include <RTClib.h> // Thư viện RTC
#include <SPI.h> // Thư viện SPI
#include <Ethernet.h>
#include <BlynkSimpleStream.h> // Blynk
#include <SoftwareSerial.h>

// RTC
RTC_DS1307 rtc; // Khai báo biến
// char daysOfTheWeek[7][12]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
// char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char daysOfTheWeek[7][12] = {"ChN", "Hai", "Ba", "Tu", "Nam", "Sau", "Bay"};

// Khởi tạo chân
LiquidCrystal lcd(9, 8, 5, 4, 3, 2); // Khởi tạo chân giao tiếp
SoftwareSerial DebugSerial(17, 16); // RX, TX

// Blynk
#define BLYNK_TEMPLATE_ID "TMPLSWy0uUgW"
#define BLYNK_DEVICE_NAME "HeThongVuonThongMinh"
#define BLYNK_AUTH_TOKEN "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk"
char auth[] = "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk";

// Khai báo chân (Hằng số)
#define pinpHSensor A0
#define motor_bomnuoc 26
// #define pinEcho 19 // Chan echo cua HCSR04
// #define pinTrigg 18 // Chan trigger cua HCSR04
#define pinWater A1 // Chan trigger cua HCSR04

// #define pinBtnEdit 23
// #define pinBtnOK 22
// #define pinBtnDOWN 21
// #define pinBtnUP 20

#define pinBtnLAMP 26
#define pinBtnMotor 25

#define pinBtnTime 24
#define pinBtnAuto 23
#define pinBtnReset 22

#define pinLightVuon 12
#define pinLDR 13
#define pinRAIN 10
#define pinMotorBom 11
#define pinMotor 7
#define pinLAMP 6

bool rainStatus = 0;
int statusLight, valueDoAmDat, giatriDAKK = 0;
float giatriNhietDo, valuepHWater;
unsigned long int avgValue;
byte nbMode = 0, statusEdit = 0, pageEdit = 0, nbModeLate = 0;
DateTime now, lastUpdateLCD;
char valuebth;
int spi_receiver = 0;
int flag;



unsigned long duration; // biến đo thời gian
float distance;           // biến lưu khoảng cách

void setup() {
  Serial.begin(9600);
  DebugSerial.begin(9600);

  if (!rtc.begin()){
   DebugSerial.println("Couldn't find RTC");
   while(1);
  }

  if (!rtc.isrunning()){
   DebugSerial.println("RTC is NOT running!");
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  lastUpdateLCD = rtc.now();

  lcd.begin(20, 4); // Khởi tạo LCD 16, 2

  // pinMode(pinLight, INPUT);
  
  /* pin INPUT */
  pinMode(pinRAIN, INPUT);
  pinMode(pinpHSensor, INPUT);
  pinMode(pinWater, INPUT);

  /* pin OUTPUT */
  pinMode(pinMotor, OUTPUT);
  pinMode(pinMotorBom, OUTPUT);
  pinMode(pinLAMP, OUTPUT);
  pinMode(pinLightVuon, OUTPUT);

  // pinMode(pinTrigg, OUTPUT);
  // pinMode(pinEcho, INPUT);

  pinMode(MOSI, OUTPUT); 
  // Mang dữ liệu từ các thiết bị SPI về arduino
  SPCR |= _BV(SPE);
  SPI.attachInterrupt();
  
  // Blynk.begin(Serial, auth);
}

ISR(SPI_STC_vect){
  spi_receiver = SPDR;
  SPDR = 1;
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
    Blynk.syncVirtual(V0, V1, V3, V4); // Phải dùng câu lệnh này, V3>>D.A_Dat
}

BLYNK_WRITE(V0){
  giatriNhietDo = param.asFloat();
}

BLYNK_WRITE(V1){
  giatriDAKK = param.asInt();
}

BLYNK_WRITE(V2){
  valueDoAmDat = param.asInt();
}

BLYNK_WRITE(V3){
  valueDoAmDat = param.asInt();
}

BLYNK_WRITE(V4){
  valuepHWater = param.asInt();
}

BLYNK_WRITE(V10){
  boolean value = param.asInt();
  digitalWrite(pinMotor, value);
}

BLYNK_WRITE(V20){
  int pin = param.asInt();
  digitalWrite(pinLightVuon, pin);
}

void ModuleRTC(){ // LAY THOI GIAN TU RTC
  now=rtc.now();
}

void PrintSerial(){
  DebugSerial.print("Mat troi: ");        
  if(statusLight == 0)
    DebugSerial.println("Toi");
  else
    DebugSerial.println("Sang");
  DebugSerial.print("Thoi tiet: ");
  if (rainStatus == 1)
    DebugSerial.println("Mua");
  else
    DebugSerial.println("Nang");
  // DebugSerial.print("Muc nuoc: ");        DebugSerial.println(distance);
  DebugSerial.println();
}

void PrintLCD(){
  lcd.clear(); // Xóa dữ liệu LCD

  lcd.setCursor(0, 0);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(", ");
  lcd.print(now.day(), DEC);
  lcd.print("/");
  lcd.print(now.month(), DEC);
  lcd.print(" ");
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  lcd.print(now.minute(), DEC);
  lcd.print(":");
  lcd.println(now.second(), DEC);

  lcd.setCursor(0,1); // Đặt con trỏ ở vị trí cột 0, hàng 0
  lcd.print("Mode: ");
  switch(nbMode){
    case 0:
      lcd.print("Cust");
      break;
    case 1:
      lcd.print("Auto");
      break;
    case 2:
      lcd.print("Time");
      break;
  }

  lcd.setCursor(0,2);
  lcd.print("Troi: ");
  if(statusLight == 1){
    lcd.print("Sang");
  } else {
    lcd.print("Toi");
  }

  lcd.setCursor(0,3);
  lcd.print("Thoi tiet: ");
  if(rainStatus == 1){
    lcd.print("Mua");
  } else {
    lcd.print("Nang");
  }

}

void sensorRAIN(){
  rainStatus = digitalRead(pinRAIN); // Có mưa là 1 
}

void sensorLight(){
  statusLight = digitalRead(pinLDR);
}

void Next(){
  PrintLCD();
  PrintSerial();
}

void phSensor(){
  // avgValue = map(analogRead(pinpHSensor), 0, 1023, 0, 14);
  avgValue = analogRead(pinpHSensor);
  // valuepHWater=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  valuepHWater=(float)avgValue*5.0/1024; //convert the analog into millivolt
  valuepHWater=3.5*valuepHWater;
  DebugSerial.println(valuepHWater);
  Blynk.virtualWrite(V4, valuepHWater);
}

void DoKhC(){
    /* Phát xung từ chân trig */
    // digitalWrite(pinTrigg,0);   // tắt chân trig
    delayMicroseconds(2);
    // digitalWrite(pinTrigg,1);   // phát xung từ chân trig
    delayMicroseconds(5);   // xung có độ dài 5 microSeconds
    // digitalWrite(pinTrigg,0);   // tắt chân trig
    
    /* Tính toán thời gian */
    // Đo độ rộng xung HIGH ở chân echo. 
    // duration = pulseIn(pinEcho,HIGH);  
    // Tính khoảng cách đến vật.
    distance = int(duration/2/29.412);

    if(distance > 30){ // Lớn hơm 30 cm sẽ bơm nước
      digitalWrite(pinMotorBom, 1);
    } else {
      digitalWrite(pinMotorBom, 0);
    }
}

void DoMucNuoc(){
  distance = analogRead(pinWater);
  if(distance < 30){ // Nhỏ hơm sẽ bơm nước
      digitalWrite(pinMotorBom, 1);
    } else {
      digitalWrite(pinMotorBom, 0);
    }
}

//--- RUN THIET BI
void RunMotor(int flag){
  // Run -motor
  sensorRAIN();
  if (rainStatus == 0 && flag == 1){
    Blynk.virtualWrite(V10, 1);
    digitalWrite(pinMotor, HIGH);
  } else {
    Blynk.virtualWrite(V10, 0);
    digitalWrite(pinMotor, LOW);
  }
}

void runLAMP(int flag){
  // Run -LAMP
  if(statusLight == 0 && flag==1)
    digitalWrite(pinLAMP, 1);
  else if (flag==0)
    digitalWrite(pinLAMP, 0);
}

//-- MODE
void CheckMode(){ //--Ok
  // DebugSerial.print("Mode: ");
  // DebugSerial.println(nbMode);
  if(digitalRead(pinBtnReset)){
    nbMode = 0;
  } else if(digitalRead(pinBtnAuto)){
    nbMode = 1;
  } else if(digitalRead(pinBtnTime)){
    nbMode = 2;
  } 
}

void CustMode(){ // --Ok
  if(digitalRead(pinBtnMotor)){
    digitalWrite(pinMotor, !digitalRead(pinMotor));
  } 
  if(digitalRead(pinBtnLAMP)){
    digitalWrite(pinLAMP, !digitalRead(pinLAMP));
  } 

}

void AutoMode(){
  if(spi_receiver == 1){
    RunMotor(flag = 1);
  } else if(spi_receiver==0) {
    RunMotor(flag = 0);
  }
  runLAMP(1);
}

void TimeMode(){

  if((now.hour()==6 && now.minute()==0)){
    runLAMP(0);
  }
  
  // CAI DAT GIO TUOI CAY HANG NGAY (BUỔI SÁNG VÀ TỐI)
  if(//(now.hour()>=12 && now.hour()<13)&&(now.minute()>=22&&now.minute()<=23)
    ((now.hour()>=7 && now.hour()<8)&&(now.minute()>=30&&now.minute()<=50))
                      ||
    ((now.hour()>=17 && now.hour()<18)&&(now.minute()>=30&&now.minute()<=50))
    ){
      RunMotor(1);
  }

  if((now.hour()==17 && now.minute()==0)){
    runLAMP(1);
  }
}

void loop() {
  ModuleRTC();
  if(now != lastUpdateLCD){
    PrintSerial();
    PrintLCD();
    lastUpdateLCD = now;
  }


  // Blynk.run();
  // DebugSerial.println(spi_receiver);
  // phSensor();
  ModuleRTC(); // Lấy dữ liệu giờ hiện tại.
  // DoKhC();
  DoMucNuoc();
  CheckMode();
  sensorLight(); 
  switch(nbMode){
    case 0: // Thủ công
      CustMode();
      break;
    case 1: // Tự động
      AutoMode();
      break;
    case 2: // Hẹn giờ
      TimeMode();
      break;
  }
}
