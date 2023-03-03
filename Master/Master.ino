// Import thư viện
#include <LiquidCrystal.h>  // Khai báo thư viện LCD / Kết nối qua SPI (Được hỗ trợ sẵn)
#include "DHT.h"
#include <RTClib.h>  // Thư viện RTC
// #include <Ethernet.h>
#include <BlynkSimpleStream.h>  // Blynk
#include <SoftwareSerial.h>

int runTuoi[4] = { 0, 0, 0, 0 };  // 0: Do am; 1: Mưa; 2: Nuoc; 3: pH

// RTC
RTC_DS1307 rtc;  // Khai báo biến
char daysOfTheWeek[7][12] = { "ChN", "Hai", "Ba", "Tu", "Nam", "Sau", "Bay" };

// Khởi tạo chân
LiquidCrystal lcd(9, 8, 5, 4, 3, 2);  // Khởi tạo chân giao tiếp
SoftwareSerial DebugSerial(17, 16);   // RX, TX

// Blynk
#define BLYNK_TEMPLATE_ID "TMPLSWy0uUgW"
#define BLYNK_DEVICE_NAME "HeThongVuonThongMinh"
#define BLYNK_AUTH_TOKEN "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk"
char auth[] = "Tts2WJWwPRTCD_a1c0OejzikcYrdszZwrKk";

// Khai báo chân (Hằng số)
#define pinDHT A0
#define pinSOIL A1
#define pinWater A2
#define pinpHSensor A3

#define pinBtnLAMP 26
#define pinBtnMotor 25

#define pinBtnTime 24
#define pinBtnAuto 23
#define pinBtnReset 22
#define pinBtnScreen1 14
#define pinBtnScreen2 15

#define pinLightVuon2 12
#define pinLDR 13
#define pinRAIN 10
#define pinMotorBom 11
#define pinMotorTuoi 7
#define pinLightVuon1 6

#define DHTTYPE DHT11
DHT dht(pinDHT, DHTTYPE);

bool rainStatus = 0;
int statusLight, valueDoAmDat, giatriDAKK = 0, waterValue = 0, soilValue = 0, Soil = 0, screenShow = 1;
float valuepHWater;
unsigned long int avgValue;
byte nbMode = 2, statusEdit = 0, pageEdit = 0, nbModeLate = 0;
DateTime now;
float humValue, Humi, tempValue, Temp;  // humValue: Giá trị độ ẩm không khí, tempValue: Giá trị độ ẩm nhiệt độ

// Khai báo các biến toàn cục
const int numReadings = 7; // Số lần đọc giá trị nhiệt độ
int readings[numReadings], matrixHumi[numReadings], matrixHumiDirt[numReadings]; // Mảng để lưu các giá trị nhiệt độ
int readIndex = 0; // Chỉ số của mảng để ghi giá trị mới
float total = 0, totalHumi = 0, totalHumiDirt=0; // Tổng của các giá trị nhiệt độ trong mảng
float average = 0, avgHumi = 0, avgHumiDirt=0; // Giá trị trung bình của các giá trị nhiệt độ trong mảng

unsigned long previousMillis = 0; // Biến để lưu thời điểm cuối cùng đọc nhiệt độ

const long interval = 60000; // Khoảng thời gian cách nhau giữa hai lần đọc (mili giây)

byte nhietdo[] = {
  B01110,
  B01010,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

void setup() {
  for (int i = 0; i < numReadings; i++) { // Khởi tạo các phần tử trong mảng readings bằng 0
    readings[i] = 0;
  }

  Serial.begin(9600);
  DebugSerial.begin(9600);
  if (!rtc.begin()) {
    DebugSerial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    DebugSerial.println("RTC is NOT running!");
  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(DateTime(2023, 3, 1, 7, 5, 0));
  // rtc.adjust(DateTime(2023, 3, 1, 7, 30, 0));
  //-- rtc.adjust(DateTime(2023, 2, 28, 7, 35, 0));
  // rtc.adjust(DateTime(2023, 2, 28, 5, 59, 59));
  // rtc.adjust(DateTime(2023, 2, 28, 16, 59, 59));

  dht.begin();
  lcd.begin(20, 4);
  lcd.createChar(1, nhietdo);

  /* pin INPUT */
  pinMode(pinRAIN, INPUT);
  pinMode(pinpHSensor, INPUT);
  pinMode(pinWater, INPUT);

  /* pin OUTPUT */
  pinMode(pinMotorTuoi, OUTPUT);
  pinMode(pinMotorBom, OUTPUT);
  pinMode(pinLightVuon1, OUTPUT);
  pinMode(pinLightVuon2, OUTPUT);

  // digitalWrite(pinLightVuon1, 1);

  Blynk.begin(Serial, auth);
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
  Blynk.syncVirtual(V0, V1, V3, V4);  // Phải dùng câu lệnh này, V3>>D.A_Dat
}

BLYNK_WRITE(V0) {
  Temp = param.asFloat();
}

BLYNK_WRITE(V1) {
  Humi = param.asInt();
}

BLYNK_WRITE(V2) {
  runTuoi[0] = param.asInt();
}

BLYNK_WRITE(V3) {
  statusLight = param.asInt();
}

BLYNK_WRITE(V4) {
  runTuoi[3] = param.asInt();
}

BLYNK_WRITE(V10) {
  boolean value = param.asInt();
  digitalWrite(pinMotorTuoi, value);
}

BLYNK_WRITE(V11) {
  boolean value = param.asInt();
  digitalWrite(pinMotorBom, value);
}

BLYNK_WRITE(V20) {
  boolean value = param.asInt();
  digitalWrite(pinLightVuon2, value);
}

BLYNK_WRITE(V21) {
  int value = param.asInt();
  digitalWrite(pinLightVuon1, value);
}

void ModuleRTC() {
  now = rtc.now();
}

void ReadSensor(unsigned long currentMillis) {
  // if (currentMillis - previousMillis >= interval) { // Nếu khoảng thời gian từ lần cuối đã vượt quá interval
  //   previousMillis = currentMillis; // Cập nhật lại biến previousMillis
  //   int reading = dht.readTemperature();
  //   int readHumi = dht.readHumidity();
  //   int readHumiDirt = map(analogRead(pinSOIL), 0, 1023, 0, 100);
  //   total -= readings[readIndex]; // Trừ đi phần tử cũ nhất trong mảng khỏi tổng
  //   totalHumi -= matrixHumi[readIndex];
  //   totalHumiDirt -= matrixHumiDirt[readIndex];
  //   readings[readIndex] = reading; // Gán phần tử mới vào vị trí readIndex trong mảng
  //   matrixHumi[readIndex] = readHumi;
  //   matrixHumiDirt[readIndex] = readHumiDirt;
  //   total += reading; // Cộng phần tử mới vào tổng
  //   totalHumi += readHumi;
  //   totalHumiDirt += readHumiDirt;
  //   readIndex++; // Tăng chỉ số readIndex lên một
  //   if (readIndex >= numReadings) { // Nếu chỉ số readIndex vượt quá kích thước của mảng
  //     readIndex = 0; // Đặt lại chỉ số readIndex về không
  //   }
  //   Temp = total / numReadings; // Tính giá trị trung bình bằng cách chia tổng cho số phần tử
  //   Humi = totalHumi / numReadings;
  //   runTuoi[0] = totalHumiDirt / numReadings;
  // }

  Humi = dht.readHumidity();
  Temp = dht.readTemperature();
  runTuoi[0] = map(analogRead(pinSOIL), 0, 1023, 0, 100);
}

// void ReadSoil() {
//   soilValue = analogRead(pinSOIL);
//   if (isnan(soilValue)) {
//     Serial.println("Read SENSOR SOIL that bai.");
//     return;
//   } else {
//     runTuoi[0] = map(analogRead(pinSOIL), 0, 1023, 0, 100);
//   }
// }

void sensorRAIN() {
  runTuoi[1] = digitalRead(pinRAIN);  // Có mưa là 1
}

void sensorLight() {
  statusLight = digitalRead(pinLDR);
}

void PrintSerial() {
  DebugSerial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  DebugSerial.print(", ");
  DebugSerial.print(now.day(), DEC);
  DebugSerial.print("/");
  DebugSerial.print(now.month(), DEC);
  DebugSerial.print(" ");
  DebugSerial.print(now.hour(), DEC);
  DebugSerial.print(":");
  DebugSerial.print(now.minute(), DEC);
  DebugSerial.print(":");
  DebugSerial.println(now.second(), DEC);

  DebugSerial.print("Stats Water: ");
  DebugSerial.println(runTuoi[2]);
  DebugSerial.print("Do pH: ");
  DebugSerial.print(runTuoi[3]);
  DebugSerial.println("pH");

  DebugSerial.print("Mode: ");
  switch (nbMode) {
    case 0:
      DebugSerial.println("Cust");
      break;
    case 1:
      DebugSerial.println("Auto");
      break;
    case 2:
      DebugSerial.println("Time");
      break;
  }

  DebugSerial.print("Troi: ");
  if (statusLight == 1) {
    DebugSerial.println("Sang");
  } else {
    DebugSerial.println("Toi");
  }

  DebugSerial.print("Thoi tiet: ");
  if (runTuoi[1] == 1) {
    DebugSerial.println("Mua");
  } else {
    DebugSerial.println("Nang");
  }

  DebugSerial.print("Nhiet do: ");
  DebugSerial.print(Temp);
  DebugSerial.println("'C");
  DebugSerial.print("Do am KK: ");
  DebugSerial.print(Humi);
  DebugSerial.println("%");
  DebugSerial.print("Do am DAT: ");
  DebugSerial.print(runTuoi[0]);
  DebugSerial.println("%");
  DebugSerial.println("");
}

void PrintLCD() {
  if (screenShow == 1) {
    lcd.clear();  // Xóa dữ liệu LCD
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

    lcd.setCursor(0, 1);
    lcd.print(F("Mode: "));
    switch (nbMode) {
      case 0:
        lcd.print(F("Cust"));
        break;
      case 1:
        lcd.print(F("Auto"));
        break;
      case 2:
        lcd.print(F("Time"));
        break;
    }

    lcd.setCursor(0, 2);
    lcd.print(F("Troi: "));
    if (statusLight == 1) lcd.print(F("Sang"));
    else lcd.print(F("Toi"));


    lcd.setCursor(0, 3);
    lcd.print(F("Thoi tiet: "));
    if (runTuoi[1] == 1) lcd.print(F("Mua"));
    else lcd.print(F("Nang"));

  } else {
    lcd.clear();  // Xóa dữ liệu LCD
    lcd.setCursor(0, 0);
    lcd.print(F("Nhiet do: "));
    lcd.print(Temp);
    lcd.write(1);
    lcd.print(F("C"));
    lcd.setCursor(0, 1);
    lcd.print(F("D.A dat: "));
    lcd.print(runTuoi[0]);
    lcd.print(F("%"));
    lcd.setCursor(0, 2);
    lcd.print(F("D.A KhKhi: "));
    lcd.print(Humi);
    lcd.print(F("%"));
    lcd.setCursor(0, 3);
    lcd.print(F("pHWater: "));
    lcd.print(runTuoi[3]);
  }
}

void Next() {
  PrintLCD();
  PrintSerial();
}

void phSensor() {
  // avgValue = map(analogRead(pinpHSensor), 0, 1023, 0, 14);
  avgValue = analogRead(pinpHSensor);
  // valuepHWater=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  valuepHWater = (float)avgValue * 5.0 / 1024;  //Chuyển đổi giá trị analog sang giá trị điện thế (millivolt) của nước bằng công thức (float) avgValue * 5.0 / 1024
  runTuoi[3] = 3.5 * valuepHWater; // Tính giá trị pH của nước bằng công thức runTuoi[3] = 3.5 * valuepHWater.
  // runTuoi[3]=7-((float(analogRead(pinpHSensor))/1024.0*5.0-1.5 )/0.15);
  Blynk.virtualWrite(V4, runTuoi[3]);
}

void readWater() {
  waterValue = analogRead(pinWater);
  if (waterValue < 100) {  // Nhỏ hơm sẽ bơm nước
    digitalWrite(pinMotorBom, 1);
    Blynk.virtualWrite(V11, 1);
  } else {
    digitalWrite(pinMotorBom, 0);
    Blynk.virtualWrite(V11, 0);
  }
  runTuoi[2] = waterValue;
}

//--- RUN THIET BI
void runMotor(int flag) {
  if (flag == 1) {
    digitalWrite(pinMotorTuoi, 1);
    Blynk.virtualWrite(V10, 1);
  } else {
    digitalWrite(pinMotorTuoi, 0);
    Blynk.virtualWrite(V10, 0);
  }
}

void runLedBULD(int flag) {
  // Run -LAMP
  if (flag == 1) {
    digitalWrite(pinLightVuon1, 1);
    Blynk.virtualWrite(V21, 1);
  } else {
    digitalWrite(pinLightVuon1, 0);
    Blynk.virtualWrite(V21, 0);
  }
}

void CheckScreen() {
  if (digitalRead(pinBtnScreen1)) {
    screenShow = 1;
  } else if (digitalRead(pinBtnScreen2)) {
    screenShow = 2;
  }
}

//-- MODE
void CheckMode() {  //--Ok
  if (digitalRead(pinBtnReset)) {
    nbMode = 0;
  } else if (digitalRead(pinBtnAuto)) {
    nbMode = 1;
  } else if (digitalRead(pinBtnTime)) {
    nbMode = 2;
  }
}

void CustMode() {  // --Ok
  if (digitalRead(pinBtnMotor)) {
    digitalWrite(pinMotorTuoi, !digitalRead(pinMotorTuoi));
  }
  if (digitalRead(pinBtnLAMP)) {
    digitalWrite(pinLightVuon1, !digitalRead(pinLightVuon1));
  }
}

void AutoMode() {
  if (runTuoi[0] <=40 && runTuoi[1] == 0 && runTuoi[3] >=4 && runTuoi[3] <=8){
    runMotor(1);
  } else
    runMotor(0);

  if(statusLight == 0)
    runLedBULD(1);
  else
    runLedBULD(0);
}

void TimeMode() {

  // CAI DAT GIO TUOI CAY HANG NGAY (BUỔI SÁNG VÀ TỐI)
  if (((now.hour() >6 && now.hour() < 8) && (now.minute() >= 0 && now.minute() < 31))
    || ((now.hour() >15 && now.hour() < 17) && (now.minute() >= 0 && now.minute() < 31))) {
    runMotor(1);
  } else {
    runMotor(0);
  }

  if ((now.hour() == 6 && now.minute() == 0)) {
    runLedBULD(0);
  }

  if ((now.hour() == 17 && now.minute() == 0)) {
    runLedBULD(1);
  }
}

void PullBlynk() {
  Blynk.virtualWrite(V0, Temp);        // Blynk - Nhiệt độ
  Blynk.virtualWrite(V1, Humi);        // Blynk - Đ.A Không khí
  Blynk.virtualWrite(V2, runTuoi[0]);  // Blynk - Đ.A Đất
}

void loop() {
  Blynk.run();
  ModuleRTC();
  ReadSensor(millis());
  phSensor();
  sensorRAIN();
  readWater();

  PullBlynk();
  PrintSerial();
  PrintLCD();

  CheckMode();
  CheckScreen();
  sensorLight();
  switch (nbMode) {
    case 0:  // Thủ công
      CustMode();
      break;
    case 1:  // Tự động
      AutoMode();
      break;
    case 2:  // Hẹn giờ
      TimeMode();
      break;
  }
}
