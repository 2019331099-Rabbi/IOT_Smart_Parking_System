#include <Wire.h> 
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
Servo myservo;
LiquidCrystal_I2C lcd(0x27,16,2);

#define ir_car1 D5
#define ir_car2 D6
#define ir_enter D8
#define ir_back  D7

char ssid[] = "FRIOT";
char password[] = "12345678";

int availableSlot = 2;
int total = 0;
int slot = 2, s1 = 0, s2 = 0;
int degree = 180;
int cnt = 0;

ESP8266WebServer server(80);

void handleGetSlotInfo()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  char buffer[50];
  sprintf(buffer, "{\"slot1\" : %d, \"slot2\" : %d}", s1, s2);
  server.send(200, "application/json", buffer);
}

void handleRoot() 
{
  String html = "<!DOCTYPE html><html><head><title>Control Relay</title></head><body>Hello World!</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(9600);
  pinMode(ir_car1, INPUT);
  pinMode(ir_car2, INPUT);
  pinMode(ir_enter, INPUT);
  pinMode(ir_back, INPUT);

  myservo.attach(D3);
  myservo.write(180);
  degree = 180;
  cnt = 0;

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  Car Parking  ");
  lcd.setCursor(0, 1);
  lcd.print("     System     ");
  delay(2000);
  lcd.clear();

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/getSlotInfo",handleGetSlotInfo);
  server.begin();     
}

void read_sensor()
{
  s1=0, s2=0;
  if(digitalRead(ir_car1) == 0) s1=1;
  if(digitalRead(ir_car2) == 0) s2=1;  
}

void printStatus() 
{
  lcd.setCursor(12, 0);
  lcd.print("SLOT");

  lcd.setCursor(0, 0);
  if (s1 == 1) {
    lcd.print("S1:Fill ");
  }
  else {
    lcd.print("S1:Empty");
  }

  lcd.setCursor(0, 1);
  if (s2 == 1) lcd.print("S2:Fill ");
  else lcd.print("S2:Empty");

  lcd.setCursor(13, 1);
  lcd.print(0);
  total = s1 + s2;
  slot = availableSlot-total;
  lcd.print(slot);
  Serial.println(slot);
}

bool isSlotEmpty()
{
  return slot;
}

bool requestingGateFromOutside()
{
  return digitalRead(ir_enter) == 0;
}

void openGate()
{
  for (int i=degree; i>= 0; i--)
  {
    degree = i;
    myservo.write(i);             
    delay(15);                     
  }
}

void closeGate()
{
  for (int i=degree; i<= 180; i++)
  {
    degree = i;
    myservo.write(i);             
    delay(15);                     
  }
}

bool requestingGateFromInside()
{
  return digitalRead(ir_back) == 0;
}

void loop() {
  read_sensor();
  if (cnt == 3)
  {
    closeGate();
    cnt = 0;    
  }
  else if(requestingGateFromInside())
  {
    openGate();
    cnt |= 1;
  }
  else if(requestingGateFromOutside())
  {
    if (isSlotEmpty())
    {
      openGate();
      cnt |= 2;
    }
    else 
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("     Sorry!     ");
      delay(2000);
      lcd.clear();
    }  
  }
  else 
    printStatus();

  server.handleClient();
}
