#include <SPI.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include "LineLib.h"
PF p;

unsigned long TO = millis();
int8_t raising = 1;
uint16_t clr = BLUE;
int8_t max_r=30;
int8_t min_r=25;
uint8_t radius = min_r;
uint16_t x = 50;
uint16_t y = 50;
int8_t xp = 7;
int8_t yp = 6;

void setup() {
    Serial.begin(9600);
    p.Init();
}

void loop() {
  if (TO<millis()) {
   //if(p.Touch_getXY()){
   // }
   p.Update();
   TO=millis()+50;
   }
  delay(10);

}
    // TouchScreen touch = TouchScreen(XP, YP, XM, YM,300);
    //TSPoint p                 = touch.getPoint();
    //Serial.print(radius); Serial.print("  "); Serial.println(clr);  
    //tft.drawCircle(x, y, 15, BLACK);
    //tft.fillCircle(200, 150, min_r, BLUE);
    //tft.fillCircle(200, 250, min_r, BLUE);
    //tft.fillCircle(200, 50, min_r, BLUE);

    //for(uint8_t y=0; y<9; y++){
    //  tft.fillCircle(300, y*35+16, 15, rand()%0xffff);
    //  }
    //tft.fillCircle(200, 50, min_r, BLUE);

//void blink2(){
//  if(x>470 or x<10) {xp*=-1; clr =rand()%0xffff;}
//  if(y>310 or y<10) {yp*=-1; clr =rand()%0xffff;}
//  x+=xp; y+=yp;
//  tft.drawCircle(x, y, 15, clr);
//  }

//void blink1(){
//      tft.drawCircle(200, 150, radius, clr);
//      tft.drawCircle(200, 250, radius, clr);
//      tft.drawCircle(200, 50, radius, clr);
//      radius += raising;
//      if (radius>max_r){raising=-1; clr=BLACK;}
//      if (radius<min_r){raising= 1; clr =rand()%0xffff;} // rand()%0xffff
//  }
