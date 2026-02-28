#include <Deneyap_OLED.h>

OLED OLED;

#define buttonPin D12

unsigned long pressStart = 0;
unsigned long lastRelease = 0;

String morseCode = "";
String text = "";

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  if (!OLED.begin(0x7A)) {
    Serial.println("I2C bağlantısı başarısız");
    while (1);
  }

  OLED.clearDisplay();
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
        Serial.println("butona basıldı");


    pressStart = millis();

    while (digitalRead(buttonPin) == LOW);

    unsigned long pressDuration = millis() - pressStart;

    if (pressDuration < 2000) {
      morseCode += ".";
      Serial.println(". basıldı");
    } else {
      morseCode += "-";
      Serial.println("- basıldı");

    }

    lastRelease = millis();
  }

  // yeni harfe geçiş (1 sn boşluk)
  if (morseCode != "" && millis() - lastRelease > 3000) {
    char letter = decodeMorse(morseCode);
    text += letter;
    morseCode = "";
    updateDisplay();
    Serial.println("yeni harfe geçildi");
  }

  // kelimelerarası boşluk (2 sn)
  if (millis() - lastRelease > 10000 && text.length() > 0) {
    text += " ";
    updateDisplay();
    lastRelease = millis();
        Serial.println("yeni kelimeye geçildi");

  }
}

char decodeMorse(String code) {
  if(code == ".-") return 'A';
  if(code == "-...") return 'B';
  if(code == "-.-.") return 'C';
  if(code == "-..") return 'D';
  if(code == ".") return 'E';
  if(code == "..-.") return 'F';
  if(code == "--.") return 'G';
  if(code == "....") return 'H';
  if(code == "..") return 'I';
  if(code == ".---") return 'J';
  if(code == "-.-") return 'K';
  if(code == ".-..") return 'L';
  if(code == "--") return 'M';
  if(code == "-.") return 'N';
  if(code == "---") return 'O';
  if(code == ".--.") return 'P';
  if(code == "--.-") return 'Q';
  if(code == ".-.") return 'R';
  if(code == "...") return 'S';
  if(code == "-") return 'T';
  if(code == "..-") return 'U';
  if(code == "...-") return 'V';
  if(code == ".--") return 'W';
  if(code == "-..-") return 'X';
  if(code == "-.--") return 'Y';
  if(code == "--..") return 'Z';

  return '?';
}

void updateDisplay() {
  OLED.clearDisplay();
  OLED.setTextXY(0, 0);
  OLED.putString(text.c_str());
}
