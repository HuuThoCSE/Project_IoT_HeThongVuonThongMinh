#include <BlynkSimpleStream.h> // Blynk
#include <DHT.h> // Thư viện DHT11

#define DHTPIN A0
#define pinSOIL A1

#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(2, 3); // RX, TX

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define BLYNK_PRINT DebugSerial
#define BLYNK_TEMPLATE_ID "TMPLSWy0uUgW"
#define BLYNK_DEVICE_NAME "HeThongVuonThongMinh"
#define BLYNK_AUTH_TOKEN "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk"
char auth[] = "Tts2WJWwPD_a1c0OejzikcYrdszZwrKk";

byte statusLight;
float valueDoAm, valueNhietDo = 0;
int valueSOIL = 0;

void setup() {
  pinMode(DHTPIN, INPUT);
  pinMode(pinSOIL, INPUT);
  // pinMode(pinLight, INPUT);

  dht.begin();

  DebugSerial.begin(9600);
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
}

void loop() {
  Blynk.run();
  valueSOIL = analogRead(pinSOIL); // CHECK - Độ ẩm đất
  valueSOIL = map(valueSOIL, 0, 1023, 0, 100);
  // statusLight = digitalRead(pinLight); // CHECK - AS

  valueDoAm = dht.readHumidity(); // CHECK - Độ ẩm không khí
  valueNhietDo = dht.readTemperature(); // CHECK - Nhiệt độ

  DebugSerial.print("Nhiet do: ");
  DebugSerial.println(valueNhietDo);
  DebugSerial.print("Do am KK: ");
  DebugSerial.println(valueDoAm);
  DebugSerial.print("Do am DAT: ");
  DebugSerial.println(valueSOIL);

  Blynk.virtualWrite(V0, valueNhietDo); // Blynk - Nhiệt độ
  Blynk.virtualWrite(V1, valueDoAm); // Blynk - Đ.A Không khí
  Blynk.virtualWrite(V2, valueSOIL); // Blynk - Đ.A Đất
  // Blynk.virtualWrite(V3, statusLight); // ANH SANG
  DebugSerial.println();
}
