/*****************************************************************************
* | File      	:   EPD_2in66g_test.c
* | Author      :   Waveshare team
* | Function    :   2.66inch e-Paper (G) test demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2023-12-20
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "EPD_Test.h"
#include "../e-Paper/EPD_2in66g.h"
#include "time.h"

extern SoftwareSerial mySerial; // RX, TX
extern MFRC522 mfrc522;  // Create MFRC522 instance.

int active = 0;
/*184*360 /4 = Imagesize  = 16560, too big we need /18 */
UWORD Imagesize = ((EPD_2IN66g_WIDTH % 4 == 0)? (EPD_2IN66g_WIDTH / 4 ): (EPD_2IN66g_WIDTH / 4 + 1)) * EPD_2IN66g_HEIGHT/25;

void EPD_2in66g_loop(void)
{
    if (Serial.available() > 0) {  // 檢查是否有來自 UART 的訊號
        String receivedData = Serial.readStringUntil('!');  // 讀取訊號直到遇到 '!'
        receivedData.trim();  // 移除前後空白
        execute(receivedData);
    } else if (mySerial.available() > 0) {  // 檢查是否有來自 UART 的訊號
        String receivedData = mySerial.readStringUntil('!');  // 讀取訊號直到遇到 '!'
        receivedData.trim();  // 移除前後空白
        execute(receivedData);
    }
}

void execute(String receivedData) {
    //Create a new image cache
    UBYTE *BlackImage;

    Debug("Received command: \r\n");
    Debug(receivedData);
    Debug("\r\n");

    // 根據收到的訊號來執行不同的操作
    if (receivedData == "init" && active == 0) {
        // open 5V
        Debug("open 5V, Module enters power consumption ...\r\n");
        DEV_Module_In();
        Debug("e-Paper Init and Clear...\r\n");
        EPD_2IN66g_Init();
        DEV_Delay_ms(2000);
        active = 1;
    } else if (receivedData == "clear" && active == 1) {
        Debug("Clearing the screen...");
        EPD_2IN66g_Clear(EPD_2IN66g_WHITE);
        DEV_Delay_ms(2000);
    } else if (receivedData == "display" && active == 1) {
        Debug("Displaying the image...");
        EPD_2IN66g_Display(gImage_2in66g);
        DEV_Delay_ms(2000);
    } else if (receivedData == "rfid" && active == 1) {
        Debug("Reading RFID...");
        SWITCH_SPI_CS();
        while(readRFID()==0);
        SWITCH_SPI_SS();
    } else if (receivedData == "sleep" && active == 1) {
        Debug("Going to sleep...");
        EPD_2IN66g_Sleep();
        DEV_Delay_ms(2000);  // important, at least 2s
        active = 0;
        // close 5V
        Debug("close 5V, Module enters 0 power consumption ...\r\n");
        DEV_Module_Exit();
    } else if (active == 1) {
        //Create a new image cache
        if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
            Debug("Failed to apply for black memory...\r\n");
            Debug(Imagesize);
            return -1;
        }
        Debug("Paint_NewImage\r\n");
        Paint_NewImage(BlackImage, EPD_2IN66g_WIDTH, EPD_2IN66g_HEIGHT/25, 0, WHITE);
        Paint_SetScale(4);
        Paint_Clear(EPD_2IN66g_BLACK);
        Paint_DrawString_EN(3, 0, receivedData.c_str(), &Font12, EPD_2IN66g_RED, EPD_2IN66g_YELLOW);
        EPD_2IN66g_Display_part(BlackImage, 0, 170, EPD_2IN66g_WIDTH, EPD_2IN66g_HEIGHT/25);
        DEV_Delay_ms(2000);
        free(BlackImage);
        BlackImage = NULL;
    } else {
        Debug("Invalid command!");
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
            
            Debug("RFID UID:  ");
            Debug(uid);  // Print UID to the serial monitor
            Debug("\r\n");
            mySerial.println(uid);
            
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