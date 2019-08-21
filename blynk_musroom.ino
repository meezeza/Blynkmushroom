#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleTimer.h>
#define RELAY1 D5
#define RELAY2 D6
#define RELAY3 D7
SimpleTimer timer;
/*------------------------------------------------------------*/
char auth[] = "YdAW13jEMXLoYiVe1RYUQOMqrqYncxr9";
/*-----------------------------------------------------------*/
Adafruit_AM2315 am2315;
float temperature, humidity, sethumi = 80, D_time;
int Auto = 0, set_P_time = 5000;
boolean flow = 0;
/*-----------------------------------------------------------*/
LiquidCrystal_I2C lcd(0x27, 16, 2);
/*------------------------------------------------------------*/
void setup()
{
  Serial.begin(9600);
  lcd.begin();
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  Blynk.begin(auth, "EDM_24G_AIS", "edm090162");
  while (!Serial) {
    delay(10);
  }
  Serial.println("AM2315 Test!");

  if (! am2315.begin()) {
    Serial.println("Sensor not found, check wiring & pullups!");
    lcd.setCursor(0, 0);
    lcd.print("Sensor not found");
    while (1);
  }
  Blynk.syncAll();
  Blynk.syncVirtual(V60);
  Blynk.syncVirtual(V61);
  Blynk.syncVirtual(V50);
}

void loop()
{
  Blynk.run();
  timer.run();
  if (! am2315.readTemperatureAndHumidity(&temperature, &humidity))
  {
    Serial.println("Failed to read data from AM2315");
    digitalWrite(RELAY2, HIGH);
    return;
  }
  digitalWrite(RELAY2, LOW);
  Serial.print("Temp *C: "); Serial.print(temperature);
  Serial.print("   Hum %: "); Serial.print(humidity);
  Serial.print("   SetHum %: "); Serial.println(sethumi);
  Showlcd();
  SetBlynk();
  Serial.print("flow = ");
  Serial.println(flow);
  Automatic();
  delay(2000);
}
void disflow()
{
  flow = 0;
}
void SetBlynk()
{
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
}
BLYNK_WRITE(V60)
{
  sethumi = param.asInt();
}
BLYNK_WRITE(V61)
{
  set_P_time = param.asInt() * 1000;
}
BLYNK_WRITE(V62)
{
  D_time = param.asInt() * 1000;
}
void Showlcd()
{
  lcd.setCursor(0, 0);
  lcd.print("temp = ");
  lcd.setCursor(0, 1);
  lcd.print("humi = ");
  lcd.setCursor(9, 0);
  lcd.print(temperature);
  lcd.setCursor(9, 1);
  lcd.print(humidity);
}
void Readhumi()
{
  WidgetLED led1(V40);
  WidgetLED led2(V41);
  if (humidity <= sethumi && Auto == 1)
  {
    led1.on();
    led2.off();
    digitalWrite(RELAY3, HIGH);
    timer.setTimeout(set_P_time, PumpOff);
  }
  else
  {
    digitalWrite(RELAY3, LOW);
  }
}
void PumpOff()
{
  WidgetLED led1(V40);
  WidgetLED led2(V41);
  led1.off();
  led2.on();
  digitalWrite(RELAY3, LOW);
}

BLYNK_WRITE(V50)
{
  int buttonStateV50 = param.asInt();
  Serial.print("buttonStateV50 = ");
  Serial.println(buttonStateV50);
  if (buttonStateV50 == 1) {
    Auto = 1;
    Readhumi;
  }
  else
  {
    digitalWrite(RELAY3, LOW);
    Auto = 0;
  }
}

void Automatic()
{
  if (Auto == 1)
  {
    Serial.println("Auto mode");
    Readhumi();
  }
}
