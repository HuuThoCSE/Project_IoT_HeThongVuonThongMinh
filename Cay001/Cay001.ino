#include <BlynkSimpleStream.h> // Blynk
#include <DHT.h> // Thư viện DHT11
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define DHTPIN A0
#define pinSOIL A1

LiquidCrystal lcd(12, 11, 7, 6, 5, 4); // Khởi tạo chân giao tiếp

SoftwareSerial DebugSerial(2, 3); // RX, TX
SoftwareSerial SerialBluetooth(9, 10); // RX, TX

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define BLYNK_PRINT DebugSerial
#define BLYNK_TEMPLATE_ID "TMPLSWy0uUgW"
#define BLYNK_DEVICE_NAME "HeThongVuonThongMinh"
#define BLYNK_AUTH_TOKEN "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk"
char auth[] = "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk";

byte statusLight = 0;
float valueDoAm, valueNhietDo = 0;
int valueSOIL = 0;
unsigned long Time = 0, timeDHT = 0;


void PrintLCD(){
  lcd.clear(); // Xóa dữ liệu LCD
  lcd.setCursor(0,0); // Đặt con trỏ ở vị trí cột 0, hàng 0
  lcd.print("T:");
  lcd.print(valueNhietDo);
  //lcd.print((char)223);
  //lcd.print("C");

  lcd.setCursor(9, 0);
  lcd.print("SOIL:");
  lcd.print(valueSOIL);
  //lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("DA:");
  lcd.print(valueDoAm);
  //lcd.print("%");
}

void PrintDebugSerial(){
  DebugSerial.print("Nhiet do: ");
  DebugSerial.println(valueNhietDo);
  DebugSerial.print("Do am KK: ");
  DebugSerial.println(valueDoAm);
  DebugSerial.print("Do am DAT: ");
  DebugSerial.println(valueSOIL);
  DebugSerial.println();
}

void setup() {
  SerialBluetooth.begin(9600);
  lcd.begin(16, 2);
  PrintLCD();

  pinMode(DHTPIN, INPUT);
  pinMode(pinSOIL, INPUT);
  // pinMode(pinLight, INPUT);

  dht.begin();

  DebugSerial.begin(9600);
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
}

void LayThongSo(){
  valueSOIL = analogRead(pinSOIL); // CHECK - Độ ẩm đất
  valueSOIL = map(valueSOIL, 0, 1023, 0, 100);
  // if(Time - timeDHT <= 1000){
    valueDoAm = dht.readHumidity(); // CHECK - Độ ẩm không khí
    valueNhietDo = dht.readTemperature(); // CHECK - Nhiệt độ
    // valueDoAm = random(100);
    // valueNhietDo = random(100);
    // valueSOIL = random(100);
    // timeDHT = Time;
  // }
}

void PullMaster(){
  SerialBluetooth.print(valueDoAm);
  SerialBluetooth.print(valueNhietDo);
  SerialBluetooth.print(valueSOIL);
}

void PullBlynk(){
  Blynk.virtualWrite(V0, valueNhietDo); // Blynk - Nhiệt độ
  Blynk.virtualWrite(V1, valueDoAm); // Blynk - Đ.A Không khí
  Blynk.virtualWrite(V2, valueSOIL); // Blynk - Đ.A Đất
  Blynk.virtualWrite(V3, statusLight); // ANH SANG
}

void Xuly(){
  // A, D o am duoi 40 gui dữ lieu
  if(valueSOIL < 40) SerialBluetooth.write('A');
}

void loop() {
  Time = millis();
  Blynk.run();
  LayThongSo();
  Xuly();
  PrintDebugSerial();
  PrintLCD();
  PullBlynk();
}
