// Import thư viện
#include <LiquidCrystal.h> // Khai báo thư viện LCD / Kết nối qua SPI (Được hỗ trợ sẵn)
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleStream.h> // Blynk
// #include <Wire.h> // RTC
// #include <RTClib.h>

// RTC
// RTC_DS1307 rtc;
// char daysOfTheWeek[7][12]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

// Khởi tạo chân
LiquidCrystal lcd(9, 8, 5, 4, 3, 2); // Khởi tạo chân giao tiếp

#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(10, 11); // RX, TX

// Blynk
#define BLYNK_TEMPLATE_ID "TMPLSWy0uUgW"
#define BLYNK_DEVICE_NAME "HeThongVuonThongMinh"
#define BLYNK_AUTH_TOKEN "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk"
char auth[] = "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk";

// Khai báo chân (Hằng số)
#define pinNhietDo A0
// #define pinLight 13
// #define pinRAIN 12
// #define motorPin 7
// #define LAMpPin 6

int statusLight, valueDoAmDat, statusRain, giatriDAKK = 0;
float giatriNhietDo;
// DateTime now;

void setup() {
  Serial.begin(9600);

  // rtc.begin();
  // if (!rtc.isrunning()){
  //   Serial.println("RTC is NOT running");
  //   rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  // }

  lcd.begin(16, 2); // Khởi tạo LCD 16, 2
  // dht.begin();

  // pinMode(pinLight, INPUT);
  // pinMode(pinRAIN, INPUT);
  // pinMode(motorPin, OUTPUT);
  // pinMode(LAMpPin, OUTPUT);

  DebugSerial.begin(9600);
  Blynk.begin(Serial, auth);
}

BLYNK_WRITE(V0){
  giatriNhietDo = param.asFloat();
  analogWrite(pinNhietDo, giatriNhietDo);
}

BLYNK_WRITE(V1){
  int giatriDAKK = param.asInt();
}

// void ModuleRTC(){
//   // LAY THOI GIAN TU RTC
//   now=rtc.now();
//   DebugSerial.print(now.year(), DEC);
//   DebugSerial.print(" ");
//   DebugSerial.print(now.month(), DEC);
//   DebugSerial.print(" ");
//   DebugSerial.print(now.day(), DEC);
//   DebugSerial.print(" ");
//   DebugSerial.print(daysOfTheWeek[now.dayOfTheWeek()]);
//   DebugSerial.print(" ");
//   DebugSerial.print(now.hour(), DEC);
//   DebugSerial.print(" ");
//   DebugSerial.print(now.minute(), DEC);
//   DebugSerial.print(" ");
//   DebugSerial.println(now.second(), DEC);
// }

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
}

void sersorRain(){
  // statusRain = digitalRead(pinRAIN);
  DebugSerial.print("Thoi tiet: ");
  if (statusRain == 1)
    DebugSerial.println("Mua.");
  else
    DebugSerial.println("Nang.");
}

// void PrintLCD(){
//   lcd.clear(); // Xóa dữ liệu LCD
//   lcd.setCursor(0,0); // Đặt con trỏ ở vị trí cột 0, hàng 0
//   lcd.print("T:");
//   lcd.print(giatriNhietDo);

//   lcd.setCursor(8, 0);
//   lcd.print("SOIL:");
//   lcd.print(valueDoAmDat);

//   lcd.setCursor(0, 1);
//   lcd.print("DA:");
//   lcd.print(giatriDAKK);
//   lcd.print("%");

//   lcd.setCursor(10, 1);
//   lcd.print("AS:");
//   lcd.print(statusLight);
// }

void loop() {
  Blynk.run();
  PrintSerial();

  // ModuleRTC();
  // DoNhietdoDoAm();
  // sersorLight();
  // DoDoAmDat();
  // sersorRain();
  // PrintLCD();

  // //CAI DAT GIO TUOI CAY HANG NGAY (BUỔI SÁNG VÀ TỐI)
  // if(((now.hour()>=7 && now.hour()<8)&&(now.minute()>=30&&now.minute()<=50))
  //                     ||
  //   ((now.hour()>=17 && now.hour()<18)&&(now.minute()>=30&&now.minute()<=50))){
  //   digitalWrite(motorPin,HIGH);
  // }
  // else
  //   digitalWrite(motorPin,LOW);

  // if(valueDoAmDat <= 45)
  //   if (statusRain == 0)
  //     digitalWrite(motorPin, HIGH);
  //   else 
  //     digitalWrite(motorPin, LOW);
  // else 
  //   digitalWrite(motorPin, LOW);
  
  // if(statusLight == 0)
  //   digitalWrite(LAMpPin, HIGH);
  // else 
  //   digitalWrite(LAMpPin, LOW);
  DebugSerial.println();
}
