#include <SPI.h>
#include "epd2in66g.h"
#include "imagedata.h"

#include <MFRC522.h>

#include <SoftwareSerial.h>

#define rfid_SS_PIN 3   // Slave select pin
#define rfid_RST_PIN 2   // Reset pin


Epd epd;

MFRC522 mfrc522(rfid_SS_PIN, rfid_RST_PIN);  // Create MFRC522 instance.

SoftwareSerial mySerial(4, 5); // RX, TX

int active = 1;

void setup() {
  Serial.begin(9600);    // Initialize serial communication
  mySerial.begin(9600);  // 用於與其他設備通信
  
  SPI.begin();           // Initialize SPI bus

  mfrc522.PCD_Init();
  Serial.println("Place your RFID tag near the reader");

  Serial.print("e-Paper init  \r\n");
  if (epd.Init() != 0) {
      Serial.print("e-Paper init failed");
      return;
  }
  Serial.print("White \r\n");
  epd.Clear(white);
  delay(2000);
  Serial.print("Initial Image \r\n");
  epd.Display_part((unsigned char*)Line_QRcode, 8, 96, 168, 168);
  delay(2000);
}


void loop() {  
  if (Serial.available() > 0) {  // 檢查是否有來自 UART 的訊號
    String receivedData = Serial.readStringUntil('!');  // 讀取訊號直到換行符
    receivedData.trim();  // 去除前後的空格和換行符
    execute(receivedData);
  } else if (mySerial.available() > 0) {  // 檢查是否有來自 UART 的訊號
    String receivedData = mySerial.readStringUntil('!');  // 讀取訊號直到換行符
    receivedData.trim();  // 去除前後的空格和換行符
    execute(receivedData);
  }
}


void execute(String receivedData) {
  Serial.print("Received command: ");
  Serial.println(receivedData);

  // 根據收到的訊號來執行不同的操作
  if (receivedData == "clear" && active == 1) {
    // 清除顯示
    epd.Clear(white);
    Serial.println("Display cleared.");
  } else if (receivedData == "show" && active == 1) {
    // 顯示圖片
    epd.Display_part((unsigned char*)Line_QRcode, 8, 96, 168, 168);  // 假設 Line_QRcode 是圖片數據
    Serial.println("Displaying QRcode.");
  } else if (receivedData == "rfid" && active == 1) {
    // rfid
    Serial.println("waiting for rfid.");
    digitalWrite(CS_PIN, HIGH); // Disable e-Paper SPI while using RFID
    digitalWrite(rfid_SS_PIN, LOW);
    while(readRFID()==0);
    digitalWrite(CS_PIN, LOW);  // Ensure e-Paper CS is back to LOW
    digitalWrite(rfid_SS_PIN, HIGH);
  }  else if (receivedData == "sleep" && active == 1) {
    // 進入休眠模式
    active = 0;
    epd.Sleep();
    Serial.println("Entering sleep mode.");
  } else if (receivedData == "wakeup" && active == 0) {
    // 恢復
    Serial.print("e-Paper init");
    if (epd.Init() != 0) {
        Serial.print("e-Paper init failed");
        return;
    }
    active = 1;
  } else {
    Serial.println("Unknown command received.");
  }
}

int readRFID() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      String uid = "";
      
      // Read UID and display it in the Serial Monitor
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }
      
      Serial.print("RFID UID: ");
      Serial.println(uid);  // Print UID to the serial monitor
      
      mfrc522.PICC_HaltA();   // Halt the card
      mfrc522.PCD_StopCrypto1(); // End encryption
      return 1;  // 成功讀取卡片
    }
  }
  else if (Serial.available() > 0) {
    return 1;
  }
  return 0;  // 沒有讀取到卡片
}
