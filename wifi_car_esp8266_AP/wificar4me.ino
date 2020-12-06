
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//----------------------------------------
const char *ssid = "WIFI_CAR";
const char *password = "wificar123";
IPAddress ip(192, 168, 4, 1);
IPAddress netmask(255, 255, 255, 0);
const int PORT = 8080;
ESP8266WebServer wServer(PORT);

//==============SENSOR PINS================================
#define PWMA     D1
#define PWMB     D2
#define DIRA     D3
#define DIRB     D4
#define MINSEED  350
#define MAXSEED  1000
//----------------------------------------
#define SSRR     D5
#define SSRL     D6
#define SERVO    D0
#define TRIG     D7
#define ECHO     D8
//----------------------------------------
Servo servo;
int SensorR = 0;  // right
int SensorL = 0;  // left
int Distance = 0;
//----------------------------------------
int modemode = 0;

//==========================================================
void Forward() {
  analogWrite(PWMA, MAXSEED);  analogWrite(PWMB, MAXSEED);
  digitalWrite(DIRA, HIGH);    digitalWrite(DIRB, HIGH);
}
void Backup() {
  analogWrite(PWMA, MAXSEED);  analogWrite(PWMB, MAXSEED);
  digitalWrite(DIRA, LOW);     digitalWrite(DIRB, LOW);
}
void Left() {
  analogWrite(PWMA, MAXSEED);  analogWrite(PWMB, MINSEED);
  //digitalWrite(DIRA, LOW);     digitalWrite(DIRB, LOW);  delay(50);
  digitalWrite(DIRA, HIGH);    digitalWrite(DIRB, LOW);  delay(100);
}
void Right() {
  analogWrite(PWMA, MINSEED);  analogWrite(PWMB, MAXSEED);
  //digitalWrite(DIRA, LOW);     digitalWrite(DIRB, LOW);  delay(50);
  digitalWrite(DIRA, LOW);     digitalWrite(DIRB, HIGH); delay(100);
}
void Stop() {
  analogWrite(PWMA, 0);        analogWrite(PWMB, 0);
}

//==========================================================
int Ping() {
  delay(50);
  digitalWrite(TRIG, LOW);    delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);   delayMicroseconds(10);
  digitalWrite(TRIG, LOW);    return int(pulseIn(ECHO, HIGH) / 58);
}
void StopScan() {
  Stop();
  unsigned int scanl, scanc, scanr;
  servo.write(50);  delay(300);  scanl = Ping();
  servo.write(150); delay(600);  scanr = Ping();
  servo.write(100); delay(300);  scanc = Ping();
  Forward();        delay(100);
}
void ForwardScan() {
  //  unsigned int distance = Ping();
  //  if (distance < 20) { //------go----go----go-------
  //    Stop();
  //    unsigned int scanl, scanc, scanr;
  //    servo.write(50);  delay(300);  scanl = Ping();
  //    servo.write(150); delay(600);  scanr = Ping();
  //    servo.write(100); delay(300);  scanc = Ping();
  //    if (scanl > scanr && scanl > scanc) Left();
  //    if (scanr > scanl && scanr > scanc) Right();
  //    delay(100);
  //  }
  Forward();
}
//=========================================================
void handleMoveRequest() {
  if (!wServer.hasArg("dir")) {
    wServer.send(404, "text / plain", "Move: undefined");
    return;
  }
  String direction = wServer.arg("dir");
  if (direction.equals("F")) {
    Forward();    wServer.send(200, "text / plain", "Move: Forward");
  } else if (direction.equals("B")) {
    Backup();     wServer.send(200, "text / plain", "Move: Backward");
  } else  if (direction.equals("S")) {
    Stop();       wServer.send(200, "text / plain", "Move: Stop");
  } else  if (direction.equals("R")) {
    Left();       wServer.send(200, "text / plain", "Turn: Left");
  } else  if (direction.equals("L")) {
    Right();      wServer.send(200, "text / plain", "Turn: Right");
  } else {
    wServer.send(404, "text / plain", "Move: undefined");
  }
}
void handleActionRequest() {
  if (!wServer.hasArg("type")) {
    wServer.send(404, "text / plain", "Action: undefined");
    return;
  }
  String type = wServer.arg("type");
  if (type.equals("1")) {
    modemode = 1;  wServer.send(200, "text / plain", "Action 1");
  } else if (type.equals("2")) {
    modemode = 2;  wServer.send(200, "text / plain", "Action 2");
  } else if (type.equals("3")) {
    modemode = 3;  wServer.send(200, "text / plain", "Action 3");
  } else if (type.equals("4")) {
    modemode = 4;  wServer.send(200, "text / plain", "Action 4");
  } else {
    modemode = 0;  wServer.send(404, "text / plain", "Action: undefined");
  }
}
void handleNotFound() {
  wServer.send(404, "text / plain", "404: Not found");
}
//=========================================================
void setup() {
  Serial.begin(115200);
  Serial.println("----------------setup------------------");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.softAP(ssid, password);
  wServer.on("/move", HTTP_GET, handleMoveRequest);
  wServer.on("/action", HTTP_GET, handleActionRequest);
  wServer.onNotFound(handleNotFound);
  wServer.begin();
  //-----------------------------------------------------
  pinMode(PWMA, OUTPUT);  pinMode(DIRA, OUTPUT);
  pinMode(PWMB, OUTPUT);  pinMode(DIRB, OUTPUT);
  pinMode(SSRL, INPUT);   pinMode(SSRR, INPUT);
  pinMode(TRIG, OUTPUT);  pinMode(ECHO, INPUT);
  servo.attach(SERVO);    servo.write(100);
}
//=========================================================
void loop() {
  Serial.println("----------------start------------------");
  if (modemode == 0) {
    wServer.handleClient();
  } else if (modemode == 1) {
    SensorL = digitalRead(SSRL);  SensorR = digitalRead(SSRR);
    //---------------------------------------------------
    if (SensorL == LOW && SensorR == LOW)    ForwardScan();
    if (SensorL == LOW && SensorR == HIGH)   Right();
    if (SensorL == HIGH && SensorR == LOW)   Left();
    if (SensorL == HIGH && SensorR == HIGH)  StopScan();
  } else {
    delay(100);
  }
}
