
#include "FS.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
String recieveText = "";
String fileText = "";

int wifiReadFlag = 0;//loop処理先頭で0に初期化、スイッチOn、またはBluetooth受信時に1
int r10 = 0;
int g10 = 10;
int b10 = 0;

#define SWITCH_PIN 12

//Wifi http Json↓
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>// JSONライブラリ
StaticJsonDocument<200> doc;

const char* ssid = "HUMAX-89B21";
const char* password = "LWRWNTNwMmXWN";

String getResponse(String url){
  String result = "";
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  WiFiClient client;
  
  HTTPClient http;
  if (!http.begin(client, url)) {
//    Serial.println("Failed HTTPClient begin!");
    return result;
  }

//  http.addHeader("Content-Type", "application/json");
  int responseCode = http.GET();
  result = http.getString();
//  Serial.println(responseCode);
//  Serial.println(result);
  http.end();
  return result;
}
//↑

void writeResponsHelloText(String url){
    String response = getResponse(url);//"http://cf239798.cloudfree.jp/led.txt");
    char Buf[50];
    response.toCharArray(Buf, 50);
    writeFile(SPIFFS, "/hello.txt", Buf);
}


//NeoPixel ↓
#include <Adafruit_NeoPixel.h>
#ifdef _AVR_
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN 25 // On Trinket or Gemma, suggest changing this to 1
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 10 // Popular NeoPixel ring size
// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 100 // Time (in milliseconds) to pause between pixels
int NUMPIXELS2=4;
//int r10 = 0;
//int g10 = 20;
//int b10 = 0;
//↑

void writeFile(fs::FS &fs, const char * path, const char * message){
Serial.printf("Writing file: %s\r\n", path);

File file = fs.open(path, FILE_WRITE);
if(!file){
Serial.println("- failed to open file for writing");
return;
}
if(file.print(message)){
Serial.println("- file written");
} else {
Serial.println("- write failed");
}
file.close();
}
/*
void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
  Serial.println("- failed to open file for reading");
  return;
  }

  Serial.println("- read from file:");
  while(file.available()){
  Serial.write(file.read());
  }
  file.close();
}
*/
void readFile(fs::FS &fs, const char * path,char *buf){
  File file = fs.open(path);
  if(!file || file.isDirectory()){
      Serial.println("- failed to open file for reading");
      return;
  }

  int i=0;
  while(file.available())
  {
    *(buf+i) = file.read();
    i++;
  }
  *(buf+i) = '\0';

  file.close();
}

void getFileLines(fs::FS &fs, const char * path){
  Serial.println("getFileLines - start");
  fileText = "";
  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }

  String line = "";
  String result = "";

  int lineSize = 33;
//  https://qiita.com/yando/items/c08fb86b8583a59e2ece
  char buffer[lineSize];
  int index = 0;
  bool hasData = false;
  Serial.println("- read from file:");
  while(file.available()){
    hasData = true;
    buffer[index] = file.read();
    index++;
    if (index >= lineSize -1) {
      break;
    }
  } 

  //終端文字を足す
  buffer[index] = '\0';

  //バッファがある場合は文字列を更新して、終端以降を除去
  if (hasData == true) {
    fileText = buffer;
    fileText.trim();
  }
  fileText = fileText + " _ size";
  delay(500);
  file.close();
  return;
}
/*
void read_json(){   
  File f = SPIFFS.open("/data.json");

  DynamicJsonDocument jsonDocument(1024);
  // deseriarizeする
  DeserializationError error = deserializeJson(jsonDocument, f);
  // deseriarizeするとFileオブジェクトに書き込めなくなるので一度開きなおす
  f.close();
//  File f = SD.open("/data.json");
}
*/
void read_json_rgb(){
  char json_buf[300];
//  hello.txt
//  readFile(SPIFFS, "/test.json",json_buf); //jsonファイル読み込み
  readFile(SPIFFS, "/hello.txt",json_buf); //jsonファイル読み込み
  Serial.println(json_buf);

  //json解析
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, json_buf);
  //Serial.println(json_buf);

  if (error) {
    // エラーの場合
    Serial.println("err");
  } else {
    // 正常な場合は値を取得して表示
    Serial.println("Noerr");
    JsonObject testa = doc["items"];
//    int testb = doc["items"][1]["y"];
    r10 = doc["items"][0][0];
    g10 = doc["items"][0][1];
    b10 = doc["items"][0][2];
    Serial.println(r10);
    Serial.println(g10);
    Serial.println(b10);
  }
}

void setup() {
  wifiReadFlag = 0;
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  // LEDのデータ線
  pinMode(LED_PIN, OUTPUT);
  // スイッチの入力
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  Serial.println("setup開始");
//  writeFile(SPIFFS, "/hello.txt", "Hello_1127");
//  writeFile(SPIFFS, "/test.json", "{""items"": [{""y"": 10,""x"": 10,""type"": ""btn"",""width"": 50,""height"": 120,""func"": ""functionA""},{""y"": 110,""x"": 50,""type"": ""btn"",""width"": 150,""height"": 20,""func"": ""functionB""}]}");
  
//  writeFile(SPIFFS, "/test.json", "{\"items\": [{\"y\": 10,\"x\": 10,\"type\": \"btn\",\"width\": 50,\"height\": 120,\"func\": \"functionA\"},{\"y\": 110,\"x\": 50,\"type\": \"btn\",\"width\": 150,\"height\": 20,\"func\": \"functionB\"}]}");
//  writeFile(SPIFFS, "/test.json", "{\"items\":[[0,10,10],[20,20,20]]}");
//  read_json_rgb();
//  getFileLines(SPIFFS, "/hello.txt");







/*
  char json_buf[300];
  readFile(SPIFFS, "/test.json",json_buf); //jsonファイル読み込み
  Serial.println(json_buf);

  //json解析
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, json_buf);
  Serial.println(json_buf);

  if (error) {
    // エラーの場合
    Serial.println("err");
  } else {
    // 正常な場合は値を取得して表示
    Serial.println("Noerr");
    JsonObject testa = doc["items"];
    int testb = doc["items"][1]["y"];
    Serial.println(testb);
  }

  Serial.println(doc["items"].size());
*/





  Serial.println("hello.txt の中身 : " + fileText);
  Serial.println("setup終了");
}

void loop() {

  pixels.clear();
  for(int i=0; i<NUMPIXELS2; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r10, g10, b10));
    pixels.show();
    delay(DELAYVAL);
  }

  if (Serial.available()) {
  // 送信 スマホに送る
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
  // 受信 ESP32が受け取りシリアルモニターに出力する
    Serial.write(SerialBT.read());
  }

  //パターン更新用のスイッチ
  if(digitalRead(SWITCH_PIN)==0){
  //スイッチが押された時の処理
    wifiReadFlag = 1;
  } else {
    wifiReadFlag = 0;
  }

  if (wifiReadFlag == 1) {
//    String response = "れすぽんす";//getResponse("http://cf239798.cloudfree.jp/led.txt");
    writeResponsHelloText("http://cf239798.cloudfree.jp/led.txt");
    read_json_rgb();

/*
    char json_buf[300];
    readFile(SPIFFS, "/test.json",json_buf); //jsonファイル読み込み
    Serial.println(json_buf);



  //json解析
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, json_buf);
//  Serial.println(json_buf);

  if (error) {
    // エラーの場合
    Serial.println("err");
  } else {
    // 正常な場合は値を取得して表示
    Serial.println("Noerr");
    JsonObject testa = doc["items"];
//    int testb = doc["items"][1]["y"];
    int testb = doc["items"][0][0];
    int testb = doc["items"][0][1];
    int testb = doc["items"][0][2];
    Serial.println(testb);
  }

    char Buf[50];
    response.toCharArray(Buf, 50);
    writeFile(SPIFFS, "/hello.txt", Buf);
    Serial.println(response);
    if (g10 == 0)
    {
      Serial.println("g10 == 0");
      r10 = 0;
      g10 = 20;
      b10 = 0;
      getFileLines(SPIFFS, "/hello.txt");  
      Serial.println("hello.txt の中身 : " + fileText);
    }
    else
    {
      Serial.println("else");
      r10 = 20;
      g10 = 20;
      b10 = 0;
      //readFile(SPIFFS, "/hello.txt");
    }
*/
    Serial.println("パターン更新用のスイッチ");
  //  getFileLines(SPIFFS, "/hello.txt");  
  //  Serial.println(fileText + "_@@");
    
  //  Serial.println(fileText);
    delay(1000);
    wifiReadFlag = 0;
  }

  delay(20);
}
