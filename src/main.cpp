#include <Arduino.h>
#include <M5Core2.h>
#include "NimBLEDevice.h"
#include <string>
#include <sstream>
#include <iostream>

////////////////////////////////////////////////
//// Variables
////////////////////////////////////////////////
// Declaring function names
void mainMenu();
void waitTurn();
void playerTurn();
void drawCirlces();
void loadGame();
bool createServer();
bool getServers();
void sendInfo(int, int);
void recieveInfo();
void hostWaitScreen();
bool checkMove(int);
void updateBoard();
void test();
int horizontalCheck(int, int);
int verticalCheck(int, int);
int diagonalDownCheck(int, int);
int diagonalUpCheck(int, int);
void endGameCheck(int);
void joinWaitScreen();

// Connection variables
NimBLEUUID serviceUuid("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
NimBLEUUID characteristicUuid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

// Server variables
NimBLEServer *pServer;
NimBLEService *pService;
NimBLECharacteristic *pCharacteristic;
NimBLEAdvertising *pAdvertising;

// Client variables
NimBLEScan *pScan;
NimBLEScanResults results;
NimBLEAdvertisedDevice device;
NimBLEClient *pClient;
NimBLERemoteService *prService;
NimBLERemoteCharacteristic *prCharacteristic;

// Variables
int deviceType;
bool turn;
bool gameOver = false;
std::string message;
int gameBoard[6][7];

// Touch button colors
ButtonColors columnColors = {BLUE, WHITE, WHITE};
ButtonColors pressedColors = {BLACK, BLACK, WHITE};
ButtonColors joinColors = {RED, WHITE, WHITE};
ButtonColors hostColors = {YELLOW, BLACK, WHITE};
ButtonColors transparentColors = {NODRAW, NODRAW, NODRAW};


////////////////////////////////////////////////
//// Main
////////////////////////////////////////////////

void setup() {
  NimBLEDevice::init("Nate's M5Core2");
  M5.begin();
  mainMenu();
}

void loop() {
  if(turn) {
    playerTurn();
  } else {
    waitTurn();
  }

  if(gameOver) {
    
  }

  delay(10);
}

////////////////////////////////////////////////
//// UI
////////////////////////////////////////////////

void mainMenu() {
  M5.Lcd.clear();
  M5.Lcd.fillScreen(TFT_BLACK);

  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(50, 50);
  M5.Lcd.print("CONNECT 4");

  M5.Lcd.setCursor(15, 130);
  M5.Lcd.print("HOST OR JOIN");

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(35, M5.Lcd.height() - 15);
  M5.Lcd.print("HOST");

  M5.Lcd.setCursor(M5.Lcd.width() - 75, M5.Lcd.height() - 15);
  M5.Lcd.print("JOIN");
  
  while(true) {
    M5.update();
    if(M5.BtnA.isPressed()) {
        createServer();
        hostWaitScreen();
        break;
    } else if (M5.BtnC.isPressed()) {
        joinWaitScreen();
        getServers();
        break;
    }
  }
  M5.begin();
  loadGame();
}

void joinWaitScreen() {
  M5.Lcd.clear();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(20, M5.Lcd.height() / 2 - 20);
  String text = "Searching for \n games to join...";
  M5.Lcd.setTextSize(3);
  M5.Lcd.print(text);
}

void hostWaitScreen() {
  M5.Lcd.clear();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(50, M5.Lcd.height() / 2 - 20);
  String text = "Waiting for \nplayer to join...";
  M5.Lcd.setTextSize(3);
  M5.Lcd.print(text);

  while(pServer->getPeerDevices().size() < 1) {
    delay(10);
  }
}

void drawCircles() {
  int radius = 14;
  int columnCoordX = 22;
  int columnCoordY = 65;
  int ROWS = 6;
  int COLUMNS = 7;
  
  M5.Lcd.clear();
  M5.Lcd.fillScreen(BLUE);

  /**
   * Create a 2d array to store player choices
   * - 1 == player one
   * - 2 == player two
   * - 0 == unused space
   * print fillCircles at each column based on num val at index i & j
   */
  for(int i = 0; i < COLUMNS; i++) {
    for(int j = 0; j < ROWS; j++) {
      if (gameBoard[j][i] == 1) {
        // PLAYER ONE --> YELLOW
        M5.Lcd.fillCircle(columnCoordX + (i * 45), columnCoordY + (j * 30), radius, YELLOW);
      } else if (gameBoard[j][i] == 2) {
        // PLAYER TWO --> RED
        M5.Lcd.fillCircle(columnCoordX + (i * 45), columnCoordY + (j * 30), radius, RED);
      } else {
        // UNUSED SPACES
        M5.Lcd.fillCircle(columnCoordX + (i * 45), columnCoordY + (j * 30), radius, WHITE);
      }
    }
  }
}

////////////////////////////////////////////////
//// Game
////////////////////////////////////////////////

void loadGame() {
  if(deviceType == 1) {
    turn = true;
  } else {
    turn = false;
  }
  gameOver = false;
  drawCircles();
}

// Check if move is legal
bool checkMove(int y) {
  for(int i = 5; i >= 0; i--) {
      // If the move is legal, update board and send info to other device
      if(gameBoard[i][y] == 0) {
          Serial.println("Move was legal");
          gameBoard[i][y] = deviceType;
          sendInfo(i, y);
          drawCircles();
          return true;
      }
  }
  Serial.println("Move was not legal");
  return false;
}

void playerTurn() {
  Button columnButtonOne(0, 40, 45, 200, false, "", columnColors, pressedColors);
  Button columnButtonTwo(45, 40, 45, 200, false, "", columnColors, pressedColors);
  Button columnButtonThree(90, 40, 45, 200, false, "", columnColors, pressedColors);
  Button columnButtonFour(135, 40, 45, 200, false, "", columnColors, pressedColors);
  Button columnButtonFive(180, 40, 45, 200, false, "", columnColors, pressedColors);
  Button columnButtonSix(225, 40, 45, 200, false, "", columnColors, pressedColors);
  Button columnButtonSeven(270, 40, 45, 200, false, "", columnColors, pressedColors);

  drawCircles();

  while(turn) {
    M5.update();
    if(M5.BtnA.wasPressed()) {
      checkMove(0);
    }

    if(columnButtonTwo.wasPressed()) {
      checkMove(1);
    }

    if(columnButtonThree.wasPressed()) {
      checkMove(2);
    }

    if(columnButtonFour.wasPressed()) {
      checkMove(3);
    }

    if(columnButtonFive.wasPressed()) {
      checkMove(4);
    }

    if(columnButtonSix.wasPressed()) {
      checkMove(5);
    }

    if(columnButtonSeven.wasPressed()) {
      checkMove(6);
    }
    delay(10);
  }
}

void waitTurn() {
  while(!turn) {
    recieveInfo();
    if(deviceType == 1 && message[2] == '1') {
        turn = true;
    } else if(deviceType == 2 && message[2] == '2') {
        turn = true;
    }
    delay(10);
  }
  updateBoard();
  drawCircles();
  test();
}

// Updates device's boars with opponent's move
void updateBoard() {
  String move1 = message.substr(0, 1).c_str();
  String move2 = message.substr(1, 1).c_str();
  int x = move1.toInt();
  int y = move2.toInt();

  if(deviceType == 1) {
      gameBoard[x][y] = 2;
  } else if(deviceType == 2) {
      gameBoard[x][y] = 1;
  }
}

void gameEndCheck(int num) {
  if(num == 1 || num == 2) {
    M5.Lcd.clear();
    if(deviceType == num) {
      M5.Lcd.fillScreen(GREEN);
      M5.Lcd.setTextSize(4);
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.setCursor(M5.Lcd.width() - 250, M5.Lcd.height() / 2);
      M5.Lcd.print("You Win!");
      delay(3000);
      ESP.restart();
    } else {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.setTextColor(RED);
      M5.Lcd.setCursor(M5.Lcd.width() - 250, M5.Lcd.height() / 2);
      M5.Lcd.print("You Lose!");
      delay(3000);
      ESP.restart();
    }
  }
}

void test() {
  for(int x = 0; x <= 5; x++) {
    for(int y = 0; y <= 6; y++) {
      if(gameBoard[x][y] == 0) {

      } else {
        gameEndCheck(horizontalCheck(x, y));
        gameEndCheck(verticalCheck(x, y));
        gameEndCheck(diagonalDownCheck(x, y));
        gameEndCheck(diagonalUpCheck(x, y));
      }
    }
  }
}

int horizontalCheck(int x, int y) {
  // Cannot have 4 in a row from this position
  if(y >= 4) {
    return 0;
  }

  for(int i = 0; i < 4; i++) {
    // Break if no matches
    if(gameBoard[x][y + i] != gameBoard[x][y]) {
      return 0;
    }
  }
  return gameBoard[x][y];
}

int verticalCheck(int x, int y) {
  // Cannot have 4 in a row from this position
  if(x >= 3) {
    return 0;
  }

  for(int i = 0; i < 4; i++) {
    // Break if no matches
    if(gameBoard[x + i][y] != gameBoard[x][y]) {
      return 0;
    }
  }
  return gameBoard[x][y];
}

int diagonalDownCheck(int x, int y) {
  // Cannot have 4 in a row from this position
  if(x >= 3 || y >= 4) {
    return 0;
  }

  for(int i = 0; i < 4; i++) {
    // Break if no matches
    if(gameBoard[x + i][y + i] != gameBoard[x][y]) {
      return 0;
    }
  }

  return gameBoard[x][y];
}

int diagonalUpCheck(int x, int y) {
  // Cannot have 4 in a row from this position
  if(x >= 3 || y <= 3) {
    return 0;
  }

  for(int i = 0; i < 4; i++) {
    // Break if no matches
    if(gameBoard[x + i][y - i] != gameBoard[x][y]) {
      return 0;
    }
  }

  return gameBoard[x][y];
}

////////////////////////////////////////////////
//// Server/Client
////////////////////////////////////////////////

void sendInfo(int x, int y) {
  // Conevrt int into string
  std::stringstream ssX;
  std::stringstream ssY;
  std::string finalX;
  std::string finalY;

  ssX << x;
  ssY << y;
  ssX >> finalX;
  ssY >> finalY;

  // Send message
  message = "";
  message.append(finalX.c_str());
  message.append(finalY.c_str());

  if(deviceType == 1) {
      message.append("2");
      pCharacteristic->setValue(message);
      Serial.println("Server sent message");
  } else if(deviceType == 2) {
      message.append("1");
      prCharacteristic->writeValue(message);
      Serial.println("Client sent message");
  }
  test();
  turn = false;
}

void recieveInfo() {
  if(deviceType == 1) {
    message = pCharacteristic->getValue().c_str();
  } else if(deviceType == 2) {
    message = prCharacteristic->readValue().c_str();
  }
}

// Creates a server to be the host
bool createServer() {
  try {
    // Creates a server
    pServer = NimBLEDevice::createServer();
    pService = pServer->createService(serviceUuid);
    pCharacteristic = pService->createCharacteristic(characteristicUuid);

    // Starts the server
    pService->start();
    pCharacteristic->setValue("Hello BLE");

    // Advertises server for clients
    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(serviceUuid);
    pAdvertising->start();

    // Return true if server creation successful
    deviceType = 1;
    return true;
  } catch (...) {
    // Return false if server creation failed
    return false;
  }
}

// Scans for a hosts servers
bool getServers() {
  try {
    // Attempts to scan for already created servers
    pScan = NimBLEDevice::getScan();
    results = pScan->start(5);

    // Iterates through servers to look for one with a specific ID
    for(int i = 0; i < results.getCount(); i++) {
      device = results.getDevice(i);

      if(device.isAdvertisingService(serviceUuid)) {
        // Creates client when desired server is found
        pClient = NimBLEDevice::createClient();

        // Checks if connection was successful
        if(pClient->connect(&device)) {
          prService = pClient->getService(serviceUuid);
          
          // Pulls and stores useful informationf for future use
          if(prService != nullptr) {
            prCharacteristic = prService->getCharacteristic(characteristicUuid);

            if(prCharacteristic != nullptr) {
              // Return true if connection successful
              deviceType = 2;
              return true;
            }
          }
        } else {
          // Return false if conneciton failed
          return false;
        }
      }
    }
  } catch (...) {
    // Return false if connection failed
    return false;
  }
}