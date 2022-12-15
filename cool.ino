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
LiquidCrystal_I2C lcd(0x27,16,2); 

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

const int data[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // " "
  0x00, 0x00, 0x21, 0x7F, 0x01, 0x00, 0x00, 0x00, // "1"
  0x00, 0x00, 0x23, 0x45, 0x49, 0x31, 0x00, 0x00, // "2"
  0x00, 0x00, 0x22, 0x49, 0x49, 0x36, 0x00, 0x00, // "3"
  0x00, 0x00, 0x0E, 0x32, 0x7F, 0x02, 0x00, 0x00, // "4"
  0x00, 0x00, 0x79, 0x49, 0x49, 0x46, 0x00, 0x00, // "5"
  0x00, 0x00, 0x3E, 0x49, 0x49, 0x26, 0x00, 0x00, // "6"
  0x00, 0x00, 0x60, 0x47, 0x48, 0x70, 0x00, 0x00, // "7"
  0x00, 0x00, 0x36, 0x49, 0x49, 0x36, 0x00, 0x00, // "8"
  0x00, 0x00, 0x32, 0x49, 0x49, 0x3E, 0x00, 0x00, // "9"
  0x00, 0x00, 0x3E, 0x41, 0x41, 0x3E, 0x00, 0x00, // "0"
  0x00, 0x00, 0x3F, 0x44, 0x44, 0x3F, 0x00, 0x00, // "A"
  0x00, 0x00, 0x7F, 0x49, 0x49, 0x36, 0x00, 0x00, // "B"
  0x00, 0x00, 0x3E, 0x41, 0x41, 0x22, 0x00, 0x00, // "C"
  0x00, 0x00, 0x7F, 0x41, 0x41, 0x3E, 0x00, 0x00, // "D"
  0x00, 0x00, 0x7F, 0x49, 0x49, 0x41, 0x00, 0x00, // "E"
  0x00, 0x00, 0x7F, 0x48, 0x48, 0x40, 0x00, 0x00  // "F"
};

///////// GAME STATE /////////
bool player1;
bool gameFinished;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Connecting to wifi");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println("Connecting to mqtt broker...");
  if(!mqtt.connect("192.168.83.247", 1883)){
    Serial.println("Failed.");
    Serial.println(mqtt.connectError());
    while(1);
  }
  Serial.println("Connected.");
  mqtt.onMessage(onMqttMessage);
  mqtt.subscribe("Paredes/P2Input");
  msg = (char*)calloc(20,20);
  irrecv.enableIRIn();        // Start the receiver

  Wire.begin(SDA, SCL);           // attach the IIC pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("");     // The print content is displayed on the LCD

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  player1 = true;
  gameFinished = false;
}

void loop() {
  mqtt.poll();

  /*
  //lcd.setCursor(0,1); // row 1 column 0
  lcd.setCursor(0,0);
  if(player1){
    lcd.print("Player 1");
  } else {
    lcd.print("Player 2");
  }
  lcd.setCursor(0,1);
  
  // put your main code here, to run repeatedly:
  if(irrecv.decode(&results)){
    player1 = !player1;
    serialPrintUint64(results.value, HEX);
    switch(results.value){
      case 0xFF30CF:  // 1
        break;
      case 0xFF18E7:  // 2
        break;
      case 0xFF7A85:  // 3
        break;
      case 0xFF10EF:  // 4
        break;
      case 0xFF38C7:  // 5
        break;
      case 0xFF5AA5:  // 6
        break;
      case 0xFF42BD:  // 7
        break;
      case 0xFF4AB5:  // 8
        break;
      case 0xFF52AD:  // 9
        break;
    }
    Serial.println("");
    irrecv.resume();
    lcd.clear();
  }
  int cols = 0x01;
  for(int j = 0; j < (sizeof(data)/sizeof(int))/8; j++){
    for(int i = 0; i < 8; i++){
      matrixRowsVal(data[(j*8)+i]);
      matrixColsVal(~cols);
      delay(100);
      matrixRowsVal(0x00);
      cols <<= 1;  
    }
  }
  */
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

void onMqttMessage(int messageSize){
  // use the Stream interface to print the contents
  int i = 0;
  while (mqtt.available() && i < messageSize) {
    msg[i] = (char)mqtt.read();
    i++;
  }
  for(; i < 20; i++){
    msg[i] = 0;
  }
  Serial.print("Received:");
  Serial.println(msg);
  if(strcmp(msg, "quit") == 0){
    gameFinished = true;
  }
  else if(strcmp(msg, "getState") == 0){
    sendMsg(gameStateToString());
  }
  else if(strcmp(msg, "resendp2turn") == 0){
    sendMsg("p2turn");
  }
  else if(strstr(msg, "input:") != NULL){
    char* msg2 = strstr(msg, ",");
    char found[3] = {0};
    found[0] = msg2[-1];
    found[1] = msg2[1];
    Serial.println(found);
  }
}

void sendMsg(char* msg){
  mqtt.beginMessage("Paredes/ESP32");
  mqtt.print(msg);
  mqtt.endMessage();
}

char* gameStateToString(){
  return "woo";
}
