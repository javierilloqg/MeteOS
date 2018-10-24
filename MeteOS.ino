//Definimos la temperatura de la CPU para Blynk
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
//

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "images.h"
#include "DHT.h"
#include "RTClib.h"
#include "BluetoothSerial.h"

RTC_DS1307 rtc;
BluetoothSerial SerialBT;
BlynkTimer timer;

#define DHTPIN 4
#define DHTTYPE DHT11

DHT sensor(DHTPIN, DHTTYPE);
SSD1306Wire display(0x3c, 21, 22);
OLEDDisplayUi ui ( &display );

int screenW = 128;
int screenH = 64;
const int boton = 5;
const int boton2 = 15;
const int centro = 19;
const int arriba = 2;
const int abajo = 18;
int buttonState = 0;
int buttonState1 = 0;
int clockCenterX = 25;
int clockCenterY = ((screenH-16)/2)+16;
int clockRadius = 23;
int ajustes = 0;
int configarranque = 0;
byte byteRead;
bool unidad = false;
int val;
int mytimeout = millis() / 1000;
char auth[] = "XXXXXX";
char ssid[] = "XXXXXX";
char pass[] = "XXXXXX";

void sendSensor()
{
  int h = sensor.readHumidity();
  int t = sensor.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
}


void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state){
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(18,0,"MeteoS");
  DateTime now = rtc.now();
  display->drawLine(0,12, screenW-1, 12);
  //display->drawXbm(110, 0, 16, 8, bateria_bits);
  //String horas = String(now.hour(), DEC);
  //String minutos = String(now.minute(), DEC);
  //if ((now.minute() < 10) && (now.hour() < 10)){
  //  display->drawString(115, 0, "0"+ horas + ":0" + minutos);
  //}
  //else {
    //display->drawString(115, 0, horas + ":" + minutos);
  //}
  //display->drawString(115, 0, horas + ":" + minutos);
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  int temperatura = sensor.readTemperature(unidad);
  String temp = String(temperatura) + " ºC";
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24); 
  if (temperatura >= 100){
   display->drawString(x+75,y+22,"-- ºC");
  }
  else {
   display->drawString(x+75,y+22,temp);
  }
  display->drawXbm(x+0,y+16, 40, 40, termometro);
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  //int tempCPU = ((temprature_sens_read() - 32 ) / 1.8);
  //String core = String(tempCPU);
  //display->setTextAlignment(TEXT_ALIGN_CENTER); 
  //display->drawString(x+64,y+25,"Temperatura interna");
  //display->drawString(x+64,y+35,core + "ºC");
  int humedad = sensor.readHumidity();
  String hum = String(humedad) + " %";
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  if (humedad >= 100){
    display->drawString(x+75,y+22,"-- %");
  }
  else {
    display->drawString(x+75,y+22,hum);
  } 
  //display->drawString(x+75,y+22,hum);
  display->drawXbm(x+0,y+16, 40, 40, humedad_bits);
}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  display->drawXbm(x + 0, y + 16, 60 , 36, ugr_bits);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+93,y+16,"Version: 0.1R");
  display->drawString(x+91,y+26,"U. de Granada");
  display->drawString(x+91,y+36,"Javier Quesada");
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  // Reloj analogico
  DateTime now = rtc.now();
  ui.disableIndicator();
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //marcas de hora
  for( int z=0; z < 360;z= z + 30 ){
  //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }
  // secundero
  float angle = now.second() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  // minutero
  angle = now.minute() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  // horas
  angle = now.hour() * 30 + int( ( now.minute() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  //Reloj digital Nimbus_Mono_L_Regular_16
  //String timenow = String(now.hour())+":"+String(now.minute())+":"+String(now.second());
  String horas = String(now.hour(), DEC);
  String minutos = String(now.minute(), DEC);
  String segundos = String(now.second(), DEC);
  String dia = String(now.day(), DEC);
  String mes = String(now.month(), DEC);
  String anio = String(now.year(), DEC);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x+90, y+20, horas + ":" + minutos + ":" + segundos);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+90, y+35, dia + "/" + mes + "/" + anio);
  display->drawString(x+90, y+45, "Miercoles");
}

void arranque(){
   ajustes = digitalRead(centro);
   if (ajustes == HIGH){
    configarranque = 1;
   }
    display.init();
    display.flipScreenVertically();
    display.drawXbm(0, 0, 128 , 59, arranque_bits);
    display.drawString(0,50,"Iniciando ...");
    Blynk.begin(auth, ssid, pass);
    display.display();
    delay(2000);
    display.clear();
    display.end();       
}

FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4 };
int frameCount = 4;
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

void setup() {
  Serial.begin(115200);
  if (! rtc.begin()) {
    digitalWrite(2, HIGH);
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  SerialBT.begin("MeteOS Core"); //Bluetooth device name
  pinMode(boton, INPUT);
  pinMode(boton2, INPUT);
  pinMode(centro, INPUT);
  pinMode(arriba, INPUT);
  pinMode(abajo, INPUT);
  pinMode(2, OUTPUT);
  timer.setInterval(1000L, sendSensor);
  arranque();
  ui.setTargetFPS(60);
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.disableAutoTransition();
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();
  display.flipScreenVertically();
  sensor.begin();
}

void loop() {
  switch (configarranque){
  case 0: {
  SerialBT.end();
   if(Blynk.connected()){
    Blynk.run();
  }
  timer.run();
  buttonState = digitalRead(boton);
  buttonState1 = digitalRead(boton2);
  if (buttonState == HIGH) {
   ui.nextFrame();
   ui.update();
  }
  if (buttonState1 == HIGH){
   ui.previousFrame();
   ui.update();
  }
  ui.update();
  break;
  }
  case 1: {
  if( SerialBT.available() ) {
  val = SerialBT.read();

  if( val == '1' )
  { 
  ESP.restart();
  }
  }
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64,0,"Programacion");
  display.drawLine(0,12, screenW-1, 12);
  display.drawXbm(60, 20, 40, 40, flash_bits);
  display.display();
  break;
    }
  }
}
