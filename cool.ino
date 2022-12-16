#include <Arduino.h>

#include <WiFi.h>
#include <ArduinoMqttClient.h>

///////// WIFI AND MQTT /////////
WiFiClient wificlient;
MqttClient mqtt(wificlient);

char ssid[] = "RiceShop";
char password[] = "password";
char* msg;

///////// LCD /////////
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SDA 14                    //Define SDA pins
#define SCL 13                    //Define SCL pins
LiquidCrystal_I2C lcd(0x27, 16, 2);

///////// IR REMOTE /////////
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
const uint16_t recvPin = 33; // Infrared receiving pin
IRrecv irrecv(recvPin);      // Create a class object used to receive class
decode_results results;       // Create a decoding results class object

///////// LED MATRIX /////////
int latchPin = 2;          // Pin connected to ST_CP of 74HC595（Pin12）
int clockPin = 4;          // Pin connected to SH_CP of 74HC595（Pin11）
int dataPin = 15;          // Pin connected to DS of 74HC595（Pin14）

const int griddata[] = {
  0x24,0x24,0xFF,0x24,0x24,0xFF,0x24,0x24 // grid  
};
///////// GAME STATE /////////
bool player1;
bool gameActive;

char board[3][3];

bool boardInput(int x, int y) {
  // check OOB
  if (x >= 3 || y >= 3 || x < 0 || y < 0) {
    return false;
  }
  if (board[x][y] == '-') {
    if (player1) {
      board[x][y] = 'x';
    } else {
      board[x][y] = 'o';
    }
    return true;
  }
  return false;
}
char* getBoardString() {
  char* gbs_output = (char*)calloc(30, 30);
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < 3; i++) {
      char* tmp;
      if (i == 0) {
        tmp = (char*)calloc(3, 4);
        tmp[0] = board[i][j];
        tmp[1] = ' ';
        tmp[2] = '|';
      } else if (i == 2) {
        tmp = (char*)calloc(2, 3);
        tmp[0] = ' ';
        tmp[1] = board[i][j];
      } else {
        tmp = (char*)calloc(4, 5);
        tmp[0] = ' ';
        tmp[1] = board[i][j];
        tmp[2] = ' ';
        tmp[3] = '|';
      }
      strcat(gbs_output, tmp);
    }
    gbs_output[strlen(gbs_output)] = '\n';
  }
  return gbs_output;
}
char gameFinished() {
  // horizontal/vertical check
  for (int i = 0; i < 3; i++) {
    // horizontal check
    if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != '-') {
      return board[0][i];
    }
    // vertical check
    if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != '-') {
      return board[i][0];
    }
  }
  // diagonal check
  if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != '-') {
    return board[0][0];
  }
  // diagonal check
  if (board[2][0] == board[1][1] && board[1][1] == board[0][2] && board[2][0] != '-') {
    return board[2][0];
  }
  // check draw
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (board[i][j] == '-') {
        return '-';
      }
    }
  }
  // no empty spaces and no winner
  return 'd';
}
char* getGamestate() {
  char gs[2] = {0};
  gs[0] = gameFinished();
  if (gs[0] == '-') {
    return getBoardString();
  }
  return gs;
}
void clearBoard() {
  for (int i = 0 ; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      board[i][j] = '-';
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  clearBoard();
  Serial.begin(115200);
  Serial.print("Connecting to wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println("Connecting to mqtt broker...");
  if (!mqtt.connect("192.168.83.247", 1883)) {
    Serial.println("Failed.");
    Serial.println(mqtt.connectError());
    while (1);
  }
  Serial.println("Connected.");
  mqtt.onMessage(onMqttMessage);
  mqtt.subscribe("Paredes/P2Input");
  msg = (char*)calloc(20, 20);
  irrecv.enableIRIn();        // Start the receiver

  Wire.begin(SDA, SCL);           // attach the IIC pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0, 0);            // Move the cursor to row 0, column 0
  lcd.print("");     // The print content is displayed on the LCD

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  player1 = true;
  gameActive = false;
}

bool validinput = false;
void loop() {
  mqtt.poll();

  //lcd.setCursor(0,1); // row 1 column 0
  lcd.setCursor(0, 0);
  if (!gameActive) {
    lcd.print("1=1P mode.");
    lcd.setCursor(0, 1);
    lcd.print("2=2P mode.");
  } else {
    if (player1) {
      lcd.print("Your turn.");
    } else {
      lcd.print("Player 2's turn.");
    }
  }

  lcd.setCursor(0, 1);
  // put your main code here, to run repeatedly:
  if (irrecv.decode(&results)) {
    serialPrintUint64(results.value, HEX);
    if (player1) {
      if (!gameActive) {
        switch (results.value) {
          case 0xFF30CF:  // 1
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("1P mode selected.");
            lcd.setCursor(0, 1);
            lcd.print("Starting...");
            sendMsg("1pmode");
            delay(4000);
            gameActive = true;
            player1 = true;
            clearBoard();
            break;
          case 0xFF18E7:  // 2
            clearBoard();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("2P mode selected.");
            lcd.setCursor(0, 1);
            lcd.print("Starting...");
            sendMsg("2pmode");
            delay(4000);
            gameActive = true;
            player1 = true;
            clearBoard();
            break;
        }
      } else {
        switch (results.value) {
          case 0xFF30CF:  // 1
            validinput = boardInput(0, 0);
            break;
          case 0xFF18E7:  // 2
            validinput = boardInput(1, 0);
            break;
          case 0xFF7A85:  // 3
            validinput = boardInput(2, 0);
            break;
          case 0xFF10EF:  // 4
            validinput = boardInput(0, 1);
            break;
          case 0xFF38C7:  // 5
            validinput = boardInput(1, 1);
            break;
          case 0xFF5AA5:  // 6
            validinput = boardInput(2, 1);
            break;
          case 0xFF42BD:  // 7
            validinput = boardInput(0, 2);
            break;
          case 0xFF4AB5:  // 8
            validinput = boardInput(1, 2);
            break;
          case 0xFF52AD:  // 9
            validinput = boardInput(2, 2);
            break;
          default:
            validinput = false;
        }
        if (validinput) {
          player1 = false;
          Serial.println("Placed x.");
          lcd.print("Placed x.");
          delay(2000);
          sendMsg("p2turn");
        } else {
          Serial.println("Invalid input.");
          lcd.print("Invalid input.");
          delay(2000);
        }
        char gs = gameFinished();
        if (gs == 'x') {
          gameActive = false;
          player1 = true;
          lcd.clear();
          lcd.print("You won!");
          sendMsg("You lose!");
          delay(2000);
        }
        if (gs == 'o') {
          gameActive = false;
          player1 = true;
          lcd.clear();
          lcd.print("You lose!");
          sendMsg("You win!");
          delay(2000);
        }
        if (gs == 'd') {
          gameActive = false;
          player1 = true;
          lcd.clear();
          lcd.print("Draw!");
          sendMsg("Draw.");
          delay(2000);
        }
      }
    }
    Serial.println("");
    irrecv.resume();
  }
  delay(200);
  //displayBoard();
  lcd.clear();
}
void displayBoard() {
  int cols = 0x01;
  for (int i = 0; i < 8; i++) {
    matrixRowsVal(griddata[i]);
    matrixColsVal(~cols);
    delay(1);
    matrixRowsVal(0x00);
    cols <<= 1;
  }
}
void matrixRowsVal(int value) {
  // make latchPin output low level
  digitalWrite(latchPin, LOW);
  // Send serial data to 74HC595
  shiftOut(dataPin, clockPin, LSBFIRST, value);
  // make latchPin output high level, then 74HC595 will update the data to parallel output
  digitalWrite(latchPin, HIGH);
}

void matrixColsVal(int value) {
  // make latchPin output low level
  digitalWrite(latchPin, LOW);
  // Send serial data to 74HC595
  shiftOut(dataPin, clockPin, MSBFIRST, value);
  // make latchPin output high level, then 74HC595 will update the data to parallel output
  digitalWrite(latchPin, HIGH);
}

void onMqttMessage(int messageSize) {
  // assemble the response
  int i = 0;
  while (mqtt.available() && i < messageSize) {
    msg[i] = (char)mqtt.read();
    i++;
  }
  for (; i < 20; i++) {
    msg[i] = 0;
  }
  Serial.print("Received:");
  Serial.println(msg);
  if (strcmp(msg, "quit") == 0) {
    gameActive = false;
    lcd.clear();
    lcd.print("Game aborted.");
    delay(2000);
  }
  else if (strcmp(msg, "getState") == 0) {
    sendMsg(getGamestate());
  }
  else if (strcmp(msg, "resendp2turn") == 0) {
    sendMsg("p2turn");
  }
  else if (strstr(msg, "input:") != NULL) {
    char* msg2 = strstr(msg, ",");
    int in_x = atoi(&msg2[-1]);
    int in_y = atoi(&msg2[1]);
    bool valid = boardInput(in_x, in_y);
    if (valid) {
      sendMsg("valid");
      player1 = true;
    } else {
      sendMsg("invalid");
    }
  }
}

void sendMsg(char* msg) {
  mqtt.beginMessage("Paredes/ESP32");
  mqtt.print(msg);
  mqtt.endMessage();
}
