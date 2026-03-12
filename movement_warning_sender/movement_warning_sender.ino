/*
 *   Not: Kartta ilk güç verildikten sonra üstündeki DURUM LEDİ yanıp sönmeye başlamaktadır. Led söndükten sonra sensör algılamaya başlamaktadır. 
 *   Bu algılayıcı I2C haberleşme protokolü ile çalışmaktadır.
 *
*/
#include <Deneyap_HareketAlgilama.h>                    // Deneyap_HareketAlgilama kütüphanesi eklenmesi
#include <Deneyap_OLED.h>
#include <DeneyapTelegram.h>                         
#include <WiFi.h>
#include <WiFiClientSecure.h>


Gesture Hareket;                                    
OLED OLED;

// WiFi ağınızın adını giriniz. (Enter your WiFi SSID here.)
const char* ssid = "WIFI_SSID";  

// WiFi şifrenizi giriniz. (Enter your WiFi password here.)
const char* password = "WIFI_PASSWORD";  

// Telegram bot tokeninizi giriniz. (Enter your Telegram bot token here.)
#define BOTtoken "BOT_TOKEN_API"  

// Telegram chat kimliğinizi giriniz. (Enter your Telegram chat ID here.)
#define CHAT_ID "CHAT_ID"  

DeneyapTelegram telegram;
WiFiClientSecure client;

void setup() {
    Serial.begin(115200);                               // Seri monitör başlatılması
    client.setInsecure();
        Serial.print("WiFi'ye bağlanılıyor (Connecting to WiFi): ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
            
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(500);
        }
            
        Serial.println("WiFi'ye bağlandı. (WiFi connected.)");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        telegram.begin(BOTtoken);
        telegram.send(CHAT_ID, "Cihaz bağlandı. (Device is Connected.)");

    if (!Hareket.begin(0x32)) {                         // begin(slaveAdress) fonksiyonu ile cihazların haberleşmesi başlatılması
        delay(3000);
        Serial.println("I2C bağlantısı başarısız ");    // I2C bağlantısı başarısız olursa seri monitore yazdırılması
        while (1);
    }

    if (!OLED.begin(0x7A)) {
        Serial.println("I2C bağlantısı başarısız");
        while (1);
    }

  }

void loop() {
    bool gestureDurum = Hareket.readGesture();          // Hareket verisinin okunması
    if (gestureDurum == 1) {
        Serial.println("HAREKET ALGILANDI");            // Hareket ALGILANDI seri monitöre yazdırılması
        OLED.setTextXY(0, 0);
        telegram.send(CHAT_ID, "WORKING");
        delay(1000);
        OLED.putString("TELEGRAMA MESAJ GONDERILDI");
        
    }
    else {
        OLED.clearDisplay();
        //Serial.println("#################");
    }
    delay(100);
}
