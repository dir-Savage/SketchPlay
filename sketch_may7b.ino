#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <Wire.h>

// ===== HARDWARE CONFIG =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C


// super mario 
#define NOTE_E7 2637
#define NOTE_G7 3136
#define NOTE_E7 2637
#define NOTE_C7 2093
#define NOTE_D7 2349
#define NOTE_G6 1568
#define NOTE_G5 784
#define NOTE_C7 2093
#define NOTE_E6 1319
#define NOTE_A6 1760
#define NOTE_B6 1976
#define NOTE_AS6 1865
#define NOTE_A6 1760
#define NOTE_G6 1568
#define NOTE_E7 2637
#define NOTE_G7 3136
#define NOTE_A7 3520
#define NOTE_F7 2794
#define NOTE_G7 3136
#define NOTE_E7 2637
#define NOTE_C7 2093
#define NOTE_D7 2349
#define NOTE_B6 1976




#define BUZZER_PIN 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Keypad configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ===== GAME ENGINE =====
enum GameMode {MENU, DINO, BIRD, PONG, INVADERS};
GameMode currentGame = MENU;
char lastKey = 0;
unsigned long lastFrame = 0;
const unsigned long FRAME_TIME = 18; // 50 FPS

// ===== AUDIO FUNCTIONS =====
void playTone(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
}


void playMarioTheme() {
  tone(BUZZER_PIN, NOTE_E7, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_E7, 100);
  delay(300);
  tone(BUZZER_PIN, NOTE_E7, 100);
  delay(300);
  tone(BUZZER_PIN, NOTE_C7, 100);
  delay(100);
  tone(BUZZER_PIN, NOTE_E7, 100);
  delay(300);
  tone(BUZZER_PIN, NOTE_G7, 100);
  delay(550);
  tone(BUZZER_PIN, NOTE_G6, 100);
  delay(550);

  tone(BUZZER_PIN, NOTE_C7, 100);
  delay(400);
  tone(BUZZER_PIN, NOTE_G6, 100);
  delay(400);
  tone(BUZZER_PIN, NOTE_E6, 100);
  delay(400);
  tone(BUZZER_PIN, NOTE_A6, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_B6, 100);
  delay(300);
  tone(BUZZER_PIN, NOTE_AS6, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_A6, 100);
  delay(300);

  tone(BUZZER_PIN, NOTE_G6, 100);
  delay(200);
  tone(BUZZER_PIN, NOTE_E7, 100);
  delay(200);
  tone(BUZZER_PIN, NOTE_G7, 100);
  delay(200);
  tone(BUZZER_PIN, NOTE_A7, 100);
  delay(300);
  tone(BUZZER_PIN, NOTE_F7, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_G7, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_E7, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_C7, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_D7, 100);
  delay(150);
  tone(BUZZER_PIN, NOTE_B6, 100);
  delay(300);

  noTone(BUZZER_PIN);
}


void playFunnyStartMusic() {
  int melody[] = {
    262, 294, 330, 349, 392, 440, 494, 523,
    0,   330, 0,  392, 0,  262, 0, 523
  };

  int noteDurations[] = {
    150, 150, 150, 150, 150, 150, 150, 300,
    50,  150, 50, 150, 50, 150, 50, 400
  };

  for (int i = 0; i < 16; i++) {
    int note = melody[i];
    int duration = noteDurations[i];
    if (note == 0) {
      noTone(BUZZER_PIN);  // pause
    } else {
      tone(BUZZER_PIN, note, duration);
    }
    delay(duration * 1.3);  // spacing between notes
  }
}


void playResultSound(bool isWin) {
  if (isWin) {
    playTone(523, 100); delay(120);
    playTone(659, 100); delay(120);
    playTone(784, 200);
  } else {
    playTone(392, 150); delay(170);
    playTone(330, 150); delay(170);
    playTone(262, 300);
  }
}

// ===== UI FUNCTIONS =====
void drawScore(int score) {
  display.setTextSize(1);
  display.setCursor(100, 0);
  display.print(F("S:"));
  display.print(score);
}

void showGameOver(int score, bool isWin = false) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 15);
  display.print(isWin ? F("YOU WIN!") : F("GAME OVER"));
  display.setTextSize(1);
  display.setCursor(30, 40);
  display.print(F("Score: "));
  display.print(score);
  display.display();
  
  playResultSound(isWin);
  delay(2000);
  currentGame = MENU;
}

// ===== GAME STATES =====
// DINO GAME
struct DinoGame {
  int dinoY = 48;
  bool jumping = false;
  int obstacleX = 128;
  int score = 0;
  bool gameOver = false;
  
  void reset() {
    dinoY = 48;
    jumping = false;
    obstacleX = 128;
    score = 0;
    gameOver = false;
  }
  
  void handleInput(char key) {
    if (key == 'A' && !jumping) {
      jumping = true;
      playTone(523, 50);
    }
  }
  
  void update() {
    if (jumping) {
      dinoY -= 3;
      if (dinoY <= 24) jumping = false;
    } else if (dinoY < 48) {
      dinoY += 3;
    }
    
    obstacleX -= 3;
    if (obstacleX < -6) {
      obstacleX = 128;
      score++;
      playTone(262, 50);
    }
    
    if (obstacleX < 18 && obstacleX > 6 && dinoY > 40) {
      gameOver = true;
      playTone(110, 200);
    }
  }
  
  void draw() {
    display.clearDisplay();
    display.fillRect(10, dinoY, 8, 8, WHITE);
    display.fillRect(obstacleX, 48, 6, 8, WHITE);
    display.drawLine(0, 56, 128, 56, WHITE);
    drawScore(score);
    display.display();
  }
} dino;

// BIRD GAME
struct BirdGame {
  int birdY = 32;
  int velocity = 0;
  int pipeX = 128;
  int gapY = 24;
  int score = 0;
  bool gameOver = false;
  
  void reset() {
    birdY = 32;
    velocity = 0;
    pipeX = 128;
    gapY = 24;
    score = 0;
    gameOver = false;
  }
  
  void handleInput(char key) {
    if (key == 'A') {
      velocity = -6;
      playTone(523, 50);
    }
  }
  
  void update() {
    velocity += 1;
    birdY += velocity / 2;
    pipeX -= 2;
    
    if (pipeX < -20) {
      pipeX = 128;
      gapY = random(16, 40);
      score++;
      playTone(262, 50);
    }
    
    if (birdY < 0 || birdY > 56 || 
        (pipeX < 18 && pipeX > 6 && 
         (birdY < gapY || birdY > gapY + 24))) {
      gameOver = true;
      playTone(110, 200);
    }
  }
  
  void draw() {
    display.clearDisplay();
    display.fillRect(10, birdY, 8, 8, WHITE);
    display.fillRect(pipeX, 0, 20, gapY, WHITE);
    display.fillRect(pipeX, gapY + 24, 20, 64 - gapY - 24, WHITE);
    drawScore(score);
    display.display();
  }
} bird;

// PONG GAME
struct PongGame {
  int paddleY = 24;
  int opponentY = 24;
  int ballX = 64;
  int ballY = 32;
  int ballVelX = 2;
  int ballVelY = 1;
  int score = 0;
  bool gameOver = false;
  
  void reset() {
    paddleY = 24;
    opponentY = 24;
    ballX = 64;
    ballY = 32;
    ballVelX = 2;
    ballVelY = 1;
    score = 0;
    gameOver = false;
  }
  
  void handleInput(char key) {
    if (key == '2' && paddleY > 0) paddleY -= 4;
    if (key == '5' && paddleY < 48) paddleY += 4;
  }
  
  void update() {
    ballX += ballVelX;
    ballY += ballVelY;
    
    if (ballY <= 0 || ballY >= 62) {
      ballVelY = -ballVelY;
      playTone(300, 50);
    }
    
    if (ballX < 0) {
      gameOver = true;
      playTone(110, 200);
    } else if (ballX > 128) {
      score++;
      ballX = 64;
      ballY = 32;
      ballVelX = -2;
      playTone(262, 50);
      if (score >= 5) gameOver = true;
    }
    
    opponentY = constrain(ballY - 8, 0, 48);
    
    if (ballX < 12 && ballY >= paddleY && ballY <= paddleY + 16) {
      ballVelX = -ballVelX;
      playTone(400, 50);
    }
    if (ballX > 116 && ballY >= opponentY && ballY <= opponentY + 16) {
      ballVelX = -ballVelX;
      playTone(400, 50);
    }
  }
  
  void draw() {
    display.clearDisplay();
    display.fillRect(4, paddleY, 4, 16, WHITE);
    display.fillRect(120, opponentY, 4, 16, WHITE);
    display.fillRect(ballX, ballY, 2, 2, WHITE);
    drawScore(score);
    display.display();
  }
} pong;

// INVADERS GAME
struct InvadersGame {
  int playerX = 64;
  bool invaders[2][4] = {{true, true, true, true}, {true, true, true, true}};
  int invaderX = 10;
  int invaderY = 10;
  int direction = 1;
  int bulletX = -1;
  int bulletY = -1;
  int score = 0;
  bool gameOver = false;
  bool isWin = false;

  void reset() {
    playerX = 64;
    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 4; j++)
        invaders[i][j] = true;
    invaderX = 10;
    invaderY = 10;
    direction = 1;
    bulletX = -1;
    bulletY = -1;
    score = 0;
    gameOver = false;
    isWin = false;
  }

  void handleInput(char key) {
    if (key == '4' && playerX > 0) playerX -= 4;
    if (key == '6' && playerX < 120) playerX += 4;
    if (key == 'A' && bulletY == -1) {
      bulletX = playerX + 4;
      bulletY = 56;
      playTone(523, 50);
    }
  }

  void update() {
    invaderX += direction;
    if (invaderX > 88 || invaderX < 0) {
      direction = -direction;
      invaderY += 8;
    }

    if (bulletY != -1) {
      bulletY -= 4;
      if (bulletY < 0) bulletY = -1;

      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
          if (invaders[i][j] &&
              bulletX >= invaderX + j * 10 &&
              bulletX <= invaderX + j * 10 + 8 &&
              bulletY <= invaderY + i * 8 + 6 &&
              bulletY >= invaderY + i * 8) {
            invaders[i][j] = false;
            bulletY = -1;
            score++;
            playTone(262, 50);
          }
        }
      }
    }

    if (invaderY + 16 >= 56) {
      gameOver = true;
    }

    int remaining = 0;
    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 4; j++)
        if (invaders[i][j]) remaining++;

    if (remaining == 0) {
      gameOver = true;
      isWin = true;
    }
  }

  void draw() {
    display.clearDisplay();
    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 4; j++)
        if (invaders[i][j])
          display.fillRect(invaderX + j * 10, invaderY + i * 8, 8, 6, WHITE);
    display.fillRect(playerX, 58, 8, 4, WHITE);
    if (bulletY != -1)
      display.fillRect(bulletX, bulletY, 2, 4, WHITE);
    drawScore(score);
    display.display();
  }
} invaders;

// ===== MAIN LOOP =====
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  display.clearDisplay();
  display.setTextColor(WHITE);
  randomSeed(analogRead(0));
}

void loop() {
  if (millis() - lastFrame < FRAME_TIME) return;
  lastFrame = millis();
  
  char key = keypad.getKey();
  if (key != NO_KEY) lastKey = key;

  switch (currentGame) {
    case MENU:
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(10, 0);  display.print(F("A: Dino"));
      display.setCursor(10, 10); display.print(F("B: Bird"));
      display.setCursor(10, 20); display.print(F("C: Pong"));
      display.setCursor(10, 30); display.print(F("D: Invaders"));
      display.display();
      if (lastKey == 'A') { dino.reset(); currentGame = DINO; }
      if (lastKey == 'B') { bird.reset(); currentGame = BIRD; }
      if (lastKey == 'C') { pong.reset(); currentGame = PONG; }
      if (lastKey == 'D') { invaders.reset(); currentGame = INVADERS; }
      lastKey = 0;
      break;
    
    case DINO:
      dino.handleInput(lastKey);
      //playMarioTheme();
      dino.update();
      dino.draw();
      if (dino.gameOver) showGameOver(dino.score);
      break;

    case BIRD:
      bird.handleInput(lastKey);
      bird.update();
      bird.draw();
      if (bird.gameOver) showGameOver(bird.score);
      break;

    case PONG:
      pong.handleInput(lastKey);
      pong.update();
      pong.draw();
      if (pong.gameOver) showGameOver(pong.score, pong.score >= 5);
      break;

    case INVADERS:
      invaders.handleInput(lastKey);
      invaders.update();
      invaders.draw();
      if (invaders.gameOver) showGameOver(invaders.score, invaders.isWin);
      break;
  }

  lastKey = 0;
}
