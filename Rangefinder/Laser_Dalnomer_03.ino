#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>

#define __KP 2                // pin для подключения кнопки-фмксатора результата
#define __CS 10                 // Cable Select для монитора
#define __DC 5                  // RegisterSelect для монитора
#define __RES 4                 // 255 - означает, что подключен к RST-контакту (на Pro-mini - не заработал)
TFT_ILI9163C tft = TFT_ILI9163C(__CS, __DC, __RES); // подключаем монитор, присваиваем ему

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

#include <NewPing.h>
#define TRIGGER_PIN 7   // желтый провод
#define ECHO_PIN 8      // синий провод
#define MAX_DISTANCE 200
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // sonar - УЗ дальномер

#include <Wire.h>     // SDA-A4, SCL-A5 (I2C)
#include <VL53L0X.h>  
VL53L0X sensor;       // Лазерный дальномер
#define HIGH_ACCURAC


float p = 3.1415926;

unsigned long tmp_time;
unsigned long last_time = 0;
int mode = 1; // mod=0- paused, mod=1-running

void isr() {
  tmp_time  = micros();
  cli();
    if (tmp_time - last_time<500000) {sei(); return;}
    
    last_time = tmp_time;
    if (mode == 1) {mode=0;}
    else {mode=1;}
    //Serial.println(mode);


  sei(); 
  //return;
  }

class trig_but{ // класс триггера
  private:
    int pin;

 public: //       attachInterrupt(irq, ISRfunc[idx], FALLING);            // датчик под напряжением (PULL_UP) при срабатывании дергается на понижение - это приводит к прерыванию
    
    trig_but(int _pin, int _mod_min=0, int _mod_max=1){pin  = _pin;}
  
  void init(){
    int8_t irq      = digitalPinToInterrupt(pin);    // переводим номер пина в прерывание, которое на нем
    pinMode (pin, INPUT);
    attachInterrupt(irq, isr, RISING);      // датчик под напряжением (PULL_UP) при срабатывании дергается на понижение - это приводит к прерыванию
    }
};

trig_but trigger(__KP); // кнопка триггер для удержания показаний

void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  trigger.init();

  sensor.setTimeout(500);
  if (!sensor.init()) { Serial.println("Failed to detect and initialize sensor!"); while (1) {}}

    #if defined LONG_RANGE
      sensor.setSignalRateLimit(0.1); // lower the return signal rate limit (default is 0.25 MCPS)
      // increase laser pulse periods (defaults are 14 and 10 PCLKs)
      sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
      sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
    #endif
    
    #if defined HIGH_SPEED
      sensor.setMeasurementTimingBudget(20000);  // reduce timing budget to 20 ms (default is about 33 ms)
    #elif defined HIGH_ACCURACY
      sensor.setMeasurementTimingBudget(200000); // increase timing budget to 200 ms
    #endif
  
    tft.begin();
    tft.clearScreen();
    tft.drawRect( 0,0, tft.width(), tft.height(), GREEN);
    TFTprn("sonic", 1, 90, 5, RED, BLACK);           // УЗ
    TFTprn("laser", 1, 90, 60, MAGENTA, BLACK);
    
}

String STR(String S, int need_len){
  int ln = S.length();
  if (ln >= need_len) return S;
  ln = need_len-ln;
  if      (ln==1) {S=S+" ";}
  else if (ln==2) {S=S+"  ";}
  else if (ln==3) {S=S+"   ";}
  else if (ln==4) {S=S+"    ";}
  else if (ln==5) {S=S+"     ";}
  return S;
  }

void loop() {

  if (mode){
    TFTprn(STR(String(sonar.ping_cm()+9)+ " cm", 6), 2, 5, 15, RED, BLACK);           // УЗ
    //  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
    TFTprn(STR(String(sensor.readRangeSingleMillimeters()+70)+ " mm", 7), 2, 5, 70, MAGENTA, BLACK);
    
    TFTprn("                ", 1, 2, 118, GREEN, BLACK);
    }
  else {TFTprn("paused", 1, 2, 118, GREEN, BLACK);}
    
  delay(200);
}

// for (int16_t x=0; x < tft.height()-1; x+=6) {}

// ==========================================================================================
// ==========================================================================================
//
// Процедура вывода текста на экран монитора
void TFTprn(String text, int textsize, int x, int y, int textcolor, int textbg) {
     tft.setTextSize(textsize);             // устанавливаем размер текста
     tft.setTextColor(textcolor, textbg);   // устанавливаем цвет текста и фон
     tft.setCursor(x,y);                    // устанавливаем курсор в нужные координаты
     tft.print(utf8rus(text));              // печатаем текст через процедуру коррекции 
                                            // русских букв
}
String utf8rus(String source) {
 int i,k; String target; unsigned char n;   char m[2] = { '0', '\0' };  
 k = source.length(); i = 0;
 while (i < k) { n = source[i]; i++;
  if (n >= 0xC0) {
   switch (n) {
    case 0xD0: { n = source[i]; i++; 
                 if (n == 0x81) { n = 0xA8; break; } 
                 if (n >= 0x90 && n <= 0xBF) n = n + 0x2F; 
                 break; }
    case 0xD1: { n = source[i]; i++; 
                 if (n == 0x91) { n = 0xB7; break; } 
                 if (n >= 0x80 && n <= 0x8F) n = n + 0x6F; 
                 break; }}
                 }
    m[0] = n; target = target + String(m); }
return target; }
