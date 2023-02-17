// Import thư viện
#include <LiquidCrystal.h> // Khai báo thư viện LCD / Kết nối qua SPI (Được hỗ trợ sẵn)
#include "Wire.h"
#include <RTClib.h> // Thư viện RTC
#include <SPI.h>
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
SoftwareSerial DebugSerial(10, 11); // RX, TX
SoftwareSerial SerialBluetooth(15, 14); // RX, TX

// Blynk
#define BLYNK_TEMPLATE_ID "TMPLSWy0uUgW"
#define BLYNK_DEVICE_NAME "HeThongVuonThongMinh"
#define BLYNK_AUTH_TOKEN "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk"
char auth[] = "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk";

// Khai báo chân (Hằng số)
#define pinNhietDo A0
#define motor_bomnuoc 26
#define pinEcho 25 // Chan echo cua HCSR04
#define pinTrigg 24 // Chan trigger cua HCSR04
#define pinBtnEdit 23
#define pinBtnOK 22
#define pinBtnDOWN 21
#define pinBtnUP 20
#define pinBtnLAMP 19
#define pinBtnMotor 18
#define pinBtnTime 17
#define pinBtnAuto 16
#define pinBtnReset 15
#define pinLightVuon 14
#define pinLDR 13
#define pinRAIN 12
#define pinMotor 7
#define pinLAMP 6

bool statusRain = 0;
int statusLight, valueDoAmDat, giatriDAKK = 0;
float giatriNhietDo;
byte nbMode = 0, statusEdit = 0, pageEdit = 0, nbModeLate = 0;
DateTime now;
char valuebth;

void setup() {
  Serial.begin(9600);
  DebugSerial.begin(9600);
  SerialBluetooth.begin(9600);

  if (!rtc.begin()){
   DebugSerial.println("Couldn't find RTC");
   while(1);
  }

  if (!rtc.isrunning()){
   DebugSerial.println("RTC is NOT running!");
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.begin(20, 4); // Khởi tạo LCD 16, 2

  // pinMode(pinLight, INPUT);
  pinMode(pinRAIN, INPUT);
  pinMode(pinMotor, OUTPUT);
  pinMode(motor_bomnuoc, OUTPUT);
  pinMode(pinLAMP, OUTPUT);
  pinMode(pinLightVuon, OUTPUT);

  pinMode(pinTrigg, OUTPUT);
  pinMode(pinEcho, INPUT);

  
  Blynk.begin(Serial, auth);
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
    Blynk.syncVirtual(V0, V1, V3); // Phải dùng câu lệnh này, V3>>D.A_Dat
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
  DebugSerial.print("Nhiet do: ");
  DebugSerial.println(giatriNhietDo);
  DebugSerial.print("D.A Khong khi: ");
  DebugSerial.println(giatriDAKK);
  DebugSerial.print("Anh sang: ");
  DebugSerial.println(statusLight);
  DebugSerial.print("Do am dat: ");
  DebugSerial.println(valueDoAmDat);
  DebugSerial.print("Sang/Toi: ");
  DebugSerial.println(statusLight);
  DebugSerial.print("Thoi tiet: ");
  if (statusRain == 1)
    DebugSerial.println("Mua.");
  else
    DebugSerial.println("Nang.");
  DebugSerial.println();
}

void PrintLCD(){
  lcd.clear(); // Xóa dữ liệu LCD
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
}

void sensorRAIN(){
  statusRain = digitalRead(pinRAIN);
}

void sensorLight(){
  statusLight = digitalRead(pinLDR);
}

void Next(){
  PrintLCD();
  PrintSerial();
}

void DoKhC(){
  float duration; // bien do thoi gian
  float distance; // bien luu khoang cach
  digitalWrite(pinTrigg, 0); // tat chan trigg
  delayMicroseconds(2);
  digitalWrite(pinTrigg, 1); // phat xung tu chan trig
  delayMicroseconds(10);
  digitalWrite(pinTrigg, 1);
  duration = pulseIn(pinEcho, 1, 30000); // do rong xung HIGH o chan echo sau 30000us khong nhan duoc xung phan xa thi tra ve 0
  distance = (duration*0.034/2);
  DebugSerial.print("Khoan Cach: ");
  DebugSerial.print(distance);
  DebugSerial.println("cm ");
}

//--- RUN THIET BI
void RunMotor(){
  // Run -motor
  if(valueDoAmDat <= 45){
    sensorRAIN();
    if (statusRain == 0){
      Blynk.virtualWrite(V10, 1);
      digitalWrite(pinMotor, HIGH);
    } else {
      Blynk.virtualWrite(V10, 0);
      digitalWrite(pinMotor, LOW);
    }
  } else {
      Blynk.virtualWrite(V10, 0);
      digitalWrite(pinMotor, LOW);
  }
}

void runLAMP(){
  // Run -LAMP
  if(statusLight == 0)
    digitalWrite(pinLAMP, 1);
  else 
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
  RunMotor();
  runLAMP();
}

void TimeMode(){
  RunMotor();
  runLAMP();
  // CAI DAT GIO TUOI CAY HANG NGAY (BUỔI SÁNG VÀ TỐI)
  if(//(now.hour()>=12 && now.hour()<13)&&(now.minute()>=22&&now.minute()<=23)
    ((now.hour()>=7 && now.hour()<8)&&(now.minute()>=30&&now.minute()<=50))
                      ||
    ((now.hour()>=17 && now.hour()<18)&&(now.minute()>=30&&now.minute()<=50))
    ){
      RunMotor();
  }
}

void loop() {
  if(SerialBluetooth.available() > 0){
    valuebth = Serial.read();
    DebugSerial.print("vle_bth: ");
    DebugSerial.println(valuebth);
    if(valuebth == 'A'){
        RunMotor();
    }
  }
  Blynk.run();
  ModuleRTC(); // Lấy dữ liệu giờ hiện tại.
  // DoKhC();
  // if(digitalRead(pinBtnEdit)){
  //   nbModeLate = nbMode; 
  //   nbMode = 3;
  // }
  CheckMode();
  sensorLight();
  PrintLCD();
  // PrintSerial();
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
