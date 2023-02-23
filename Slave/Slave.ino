#include <BlynkSimpleStream.h> // Blynk
#include <DHT.h> // Thư viện DHT11
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "SPI.h"

#define DHTPIN A0
#define pinSOIL A1

LiquidCrystal_I2C lcd(0x27,20,4);

SoftwareSerial DebugSerial(2, 3); // RX, TX
SoftwareSerial SerialBluetooth(9, 10); // RX, TX

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define BLYNK_PRINT DebugSerial
#define BLYNK_TEMPLATE_ID "TMPLSWy0uUgW"
#define BLYNK_DEVICE_NAME "HeThongVuonThongMinh"
#define BLYNK_AUTH_TOKEN "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk"
char auth[] = "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk";


float humValue = 0,Humi = 0, tempValue = 0, Temp = 0; // humValue: Giá trị độ ẩm không khí, tempValue: Giá trị độ ẩm nhiệt độ
int soilValue = 0, Soil = 0;

// Khai báo biến cho việc giới hạn thời gian cập nhật
unsigned long previousMillis = 0; // Lưu trữ thời gian trước đó
const long interval = 5000; // Thời gian giữa các lần hiển thị trên màn hình LCD (5 giây)
const long interval2 = 3000; // Thời gian giữa các lần đọc cảm biến DHT11 (3 giây)

int spi_receiver = 0;

void PrintLCD(){
  lcd.backlight(); // Xóa dữ liệu LCD
  lcd.setCursor(0,0);     lcd.print("T:");      lcd.print(Temp);
  lcd.setCursor(9, 0);    lcd.print("SOIL:");   lcd.print(Soil);
  lcd.setCursor(0, 1);    lcd.print("DA:");     lcd.print(Humi);
}

void PrintDebugSerial(){
  DebugSerial.print("Nhiet do: ");    DebugSerial.println(Temp);
  DebugSerial.print("Do am KK: ");    DebugSerial.println(Humi);
  DebugSerial.print("Do am DAT: ");   DebugSerial.println(Soil);
  DebugSerial.println();
}

void setup() {
  lcd.init();
  PrintLCD();

  pinMode(DHTPIN, INPUT);
  pinMode(pinSOIL, INPUT);

  pinMode(MISO, OUTPUT); // 
  pinMode(SS, OUTPUT); // Chọn thiết bị SPI cần làm việc

  dht.begin();
  
  SPI.begin();
  digitalWrite(SS, HIGH); 

  // SPI.setBitOrder(LSBFIRST);

  DebugSerial.begin(9600);
  Serial.begin(9600);
  // Blynk.begin(Serial, auth);
}

void PullBlynk(){
  Blynk.virtualWrite(V0, Temp); // Blynk - Nhiệt độ
  Blynk.virtualWrite(V1, Humi); // Blynk - Đ.A Không khí
  Blynk.virtualWrite(V2, Soil); // Blynk - Đ.A Đất
}

void Xuly(){
  ///Dung SPI
  if(soilValue < 40){
    digitalWrite(SS, LOW);
    spi_receiver = SPI.transfer(1);
    digitalWrite(SS, HIGH);
  } else {
    digitalWrite(SS, LOW);
    spi_receiver = SPI.transfer(0);
    digitalWrite(SS, HIGH);
  }
}

void ReadDHTl1(){
  humValue = dht.readHumidity(); //read Humi - Do am
  tempValue = dht.readTemperature(); //read temperature - Nhiet do
  if (isnan(humValue) || isnan(tempValue)) {
    Serial.println("Failed to read fromDHT sensor!");
    return;
  } else {
    Humi = humValue;
    Temp = tempValue;
  }
}

void ReadSoil(){
  soilValue = analogRead(pinSOIL);
  if(isnan(soilValue)){
    Serial.println("Read SENSOR SOIL that bai.");
  return;
  } else {
    Soil = map(soilValue, 0, 1023, 0, 100);
  }
}


void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Đọc nhiệt độ và độ ẩm từ cảm biến DHT11
    if (currentMillis % interval2 == 0) {
      ReadDHTl1();
      ReadSoil();
      // PullBlynk();
      Xuly();
    }
    PrintDebugSerial();
    PrintLCD();
  }
  // DebugSerial.println(spi_receiver);
}
