#include <PIR.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   9
#define SS_PIN    10
#define LED_PIN   4
#define BUZZER_PIN 3

PIR sensor;

MFRC522 mfrc522(SS_PIN, RST_PIN);

bool alarmActive = false;
bool buzzerActive = false;
bool wrongCard = false;

unsigned long lastAlarmTime = 0;

void buzzerLoop() {
  unsigned long startTime = millis();
  while (millis() - startTime < 7500 && buzzerActive && (alarmActive || wrongCard)) {
    analogWrite(BUZZER_PIN, 250);
    delay(300);
    analogWrite(BUZZER_PIN, 0);
    delay(300);
    lastAlarmTime = millis();
  }
  noTone(BUZZER_PIN);
  buzzerActive = false;
  wrongCard = false;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  sensor.add(2);
  sensor.add(7);
  mfrc522.PCD_Init();
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  alarmActive = false;
}

void loop() {
  uint8_t motion = sensor.read();

  if (!alarmActive && !wrongCard && buzzerActive && millis() - lastAlarmTime > 10000) {
    noTone(BUZZER_PIN);
    buzzerActive = false;
  }
  
  if (alarmActive && motion != 0) {
    buzzerActive = true;
    buzzerLoop();
  }

  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      if (mfrc522.uid.uidByte[0] == 0xEA && mfrc522.uid.uidByte[1] == 0xAD && mfrc522.uid.uidByte[2] == 0x16 && mfrc522.uid.uidByte[3] == 0x0B) {
        alarmActive = !alarmActive;
        digitalWrite(LED_PIN, alarmActive ? HIGH : LOW);
        lastAlarmTime = millis();
      } else {
        wrongCard = true;
        buzzerActive = true;
        buzzerLoop();
      }
    }
  }
}