#include <Deneyap_KumandaKolu.h>
#include <Deneyap_5x7LedEkran.h>

DotMatrix LEDMatrix;
Joystick joystick;

int paddleX = 3;     // paddle konumu
int paddleY = 7;     // alt satır

int ballX = 3;
int ballY = 5;

int ballDX = 1;
int ballDY = -1;

void setup() {

  Serial.begin(1000000);
  joystick.begin(0x1B);

  if (!LEDMatrix.begin(0x0A)) {
    Serial.println("I2C bağlantısı başarısız");
    while (1);
  }
}

void loop() {

  uint16_t xValue = joystick.xRead();

  movePaddle(xValue);
  moveBall();
  drawGame();

  delay(150);
}

void movePaddle(uint16_t x){

  if (x < 300) {       // sağ
    if (paddleX < 5) paddleX++;
  }

  if (x > 700) {       // sol
    if (paddleX > 1) paddleX--;
  }
}

void moveBall(){

  ballX += ballDX;
  ballY += ballDY;

  // sağ sol duvar
  if (ballX <= 1 || ballX >= 5){
    ballDX = -ballDX;
  }

  // üst duvar
  if (ballY <= 1){
    ballDY = -ballDY;
  }

  // paddle çarpması
  if (ballY == paddleY-1 && ballX == paddleX){
    ballDY = -ballDY;
  }

  // top düşerse reset
  if (ballY > 7){
    ballX = 3;
    ballY = 5;
  }
}

void drawGame(){

  LEDMatrix.resetDotRows();
  // paddle
  LEDMatrix.pixelLed(paddleY, paddleX);
  // ball
  LEDMatrix.pixelLed(ballY, ballX);
}