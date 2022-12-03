// last update: 05 July 2019

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <Shift595.h>



#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


int latchPin595 = 16; // used for 595 access
int clockPin595 = 0;  //// =>> gpio0  - D3 <<=D5-gpio14
int dataPin595 = 12;   //// =>> gpio12 - D6 <<=D7-gpio13
int numOfRegisters = 1;

static int tah_pin = 2;
static int spd_pin = 4; //4-ESP // 3-MEGA
// gpio13, gpio14 - TFT_SPI

#define TFT_CS     15 //15-ESP //53-mega //5-UNO      CS_CARD=20  MISO=50 =>> pin10-UNO, pin53-MEGA = change!
#define TFT_RST    255
#define TFT_DC     5 //5-ESP  //23-mega //6-UNO     
// Option 1 (recommended): must use the hardware SPI pins (for UNO: sclk=13 and MOSI=11) and pin 10 must be an output. This is much faster - also required if you want to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);  // Option 2: use any pins but a little slower! //#define TFT_SCLK 52 - Mega2560  // 13=set these to be whatever pins you like!  //#define TFT_MOSI 51 - Mega2560  // 11=set these to be whatever pins you like!  //Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

Shift595 Shifter(dataPin595, latchPin595, clockPin595, numOfRegisters);

//
// Func     GPIO  Pin
//595_CLK  =  0   =D3
//595_MOSI = 12   =D6
//595_CS   = 16   =D0

//TFT_DC   =  5   =D1
//TFT_MOSI = 13   =D7
//TFT_CLK  = 14   =D5
//TFT_CS   = 15   =D8

//TAH      =  2   =D4
//SPD      =  4   =D2


#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define YELLOW  0xFFE0
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F

float analog_voltage_part = 3.29/1023 ;
float resistor_divider = 5.6 ;

// Generator vars
   unsigned long gen_check_print_timer = 0; //>>mills() 
   float         gen_last_value = 0.0;
   float         gen_cur_value  = 0.0;
   int           gen_round_check_period = 1000;
// Fuel vars
   unsigned long fuel_check_print_timer = 0; //>>mills() 
   float         fuel_last_value = 0.0;
   float         fuel_cur_value  = 0.0;
   int           fuel_round_check_period = 1000;

   byte        AnalogPin595 = 0;

   int           fuel_show = 0;
   float         fuel_tmp = 0;
   float FD[] = {  2.2,  2.18,  2.16,  2.14,  2.12,    2.1,   2.08,   2.06,   2.04,   2.02,  1.999,  1.977,  1.952,  1.925,  1.895,  1.863,  1.828,   1.79,  1.680,   1.53,   1.31};
   float FF[] = {0.000, 2.125, 4.250, 6.375, 8.500, 10.625, 12.750, 14.875, 17.000, 19.125, 21.250, 23.375, 25.500, 27.625, 29.750, 31.875, 34.000, 36.125, 41.225, 46.325, 51.625};
   int           fuel_max = sizeof(FD)/sizeof(float) - 1; // максимальный индекс FD

//{  0.0, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125, 2.125,   5.1,   5.1,   5.3
//
// System setup data
//
const float wheel = 1.99;            // периметр колеса в метрах
const float reduction_ratio = 1.73; // коэфициент редукции. колесо дайтчика в N раз меньше по диаметру. надо делить показания

//float tah_reduction_multiply = (float)(60000.0 / reduction_ratio); // 60'000 милисекунд в минуте. вычисляем коэфициент заранее
const float tah_reduction_multiply = 34682.1; // вычислено руками

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SPD:

    float      spd_kmh = 0;        // скорость движения
    float  old_spd_kmh = 1;        // скорость движения (последние старые показания)
    float      spd_distance = 0.0; // замер общей длины пути
    int        spd_clr = 0xF800;   // выбор цвета отображения показаний спидометра

/////////////////////////////////////
// TAH:

    int        tah_rpm = 0;        // количество оборотов в минуту
    int    old_tah_rpm = 1;        // количество оборотов в минуту (последние старые показания)
    int        tah_clr = 0xF800;   // выбор цвета отображения показаний скорости

/////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  Полный оборот колеса (tick). 
//      все формата micros()-  1/1'000'000 sec
//      регистрация изменений из тела программы - НЕ из прерывания
// SPD:

    volatile unsigned long  spd_tick_delta_time     = 0;  // ISR: -DELTA- TIME   (micros) между текущим и предыдущим TICKS
    unsigned long           old_tick_delta_time     = 0;  //      предыдущее значение (micros)
    
    volatile unsigned long  spd_last_time           = 0;  // ISR: -LAST-TICK-TIME- (micros) время последнего срабатывания
    unsigned long           tmp_spd_last_time       = 0;  //      пердыдущее значение (micros)
        
    volatile unsigned long  tmp_spd_timer           = 0;  // ISR: времянка

    unsigned long           tmp_spd_tick_delta_time = 0;  //
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// SPD - ROUND-COUNT-PER-TIME
//
    int                         spd_round_check_period = 500;   // период проверки и сброса количества оборотов колеса
    unsigned long               spd_check_print_timer  =   0;   // таймер последней проверки периода spd_round_check_period
    unsigned long           tmp_spd_millis             =   0;   // времянка
    
    volatile int                spd_rounds_counter     =   0;   // ISR- переменная регистратор оборотов
    int                     tmp_spd_rounds_counter     =   0;   // времянка переброса значения переменной
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// TAH - ROUND-COUNT-PER-TIME
//    
    int                         tah_round_check_period = 500;
    unsigned long               tah_check_print_timer  =   0;
    unsigned long           tmp_tah_millis             =   0;

    volatile int                tah_rounds_counter     =   0; // this value is checked time to time
    int                     tmp_tah_rounds_counter     =   0;
//

    int last_bar = 0;
    int bg_color = ST7735_BLACK; // цвет фона для всех отображений

// ==========================================================================================
// ==========================================================================================
//
//
//      SETUP
//
//
      void setup(void) {                                          
        Serial.begin(115200);
        Shifter.clearRegisters();
        pinMode(latchPin595, OUTPUT);
        pinMode(clockPin595, OUTPUT);
        pinMode( dataPin595, OUTPUT);
        pinMode(         A0, INPUT);
        digitalWrite(latchPin595, HIGH);
//
        tft.initR(INITR_BLACKTAB);    // (INITR_BLACKTAB) 1.8"TFT 128x160 //(INITR_144GREENTAB) 1.44"TFT 128x128  //(INITR_MINI160x80); 0.96"TFT 180x60
        tft.setRotation(3); tft.fillScreen(bg_color);
        pinMode(tah_pin, INPUT_PULLUP);     pinMode(spd_pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(tah_pin), isr_tah_rounds_counter, FALLING);   // 0-mega
        attachInterrupt(digitalPinToInterrupt(spd_pin), isr_spd_counter, FALLING);          // 1-mega
        tft.setTextWrap(true);

              TFTprn("Производится регистрация  водителя...", 2, 2, 40, ST7735_CYAN, bg_color);
              for (int16_t i=5; i > 0; i-=1) {
               TFTprn(String(i), 3, 2, 5, ST7735_CYAN, bg_color);
                delay(1000);
              }
              tft.fillScreen(bg_color);
             TFTprn("Данные       получены и   отосланы     успешно...", 2, 2, 10, ST7735_CYAN, bg_color);
              delay(2000);
              tft.fillScreen(bg_color);
        tft.drawRect(109, 3, 49, 107, ST7735_WHITE);

      }
// 13.816=100kmh  // 11.05 = 80km\h // 8.29 об\мин = 60км\ч при радиусе 201см // 4.15 => 30 км\ч // 1.38 => 10 км\ч // tah- холостой ход 175
// ================================================================================================
// ====================================== LOOP ====================================================
//
//
void loop() {
  //
  // Tahometr periodical rounds check.  tah_round_check_period=500 , every 0.5 sec.
  //
      if ((millis() - tah_check_print_timer) > tah_round_check_period){
              void_tah_rounds_check(); } 
    
      if ((millis() - spd_check_print_timer) > spd_round_check_period){
              void_spd_rounds_check(); }
    
       void_spd_ticks_detect(); // проверка регистрации нового TICK

      if ((millis() - gen_check_print_timer) > gen_round_check_period){
              void_gen_voltage_check(); } 

      if ((millis() - fuel_check_print_timer) > fuel_round_check_period){
              void_fuel_check(); } 
      
  } // loop end =============================================================================
//===========================================================================================
// for (byte i = 0; i < 8; i++) {
//  noInterrupts();
//  interrupts  ();
//  return analogRead(A0) * analog_voltage_part * resistor_divider;


void void_fuel_check() {           // brown  - 0 - 10.43 - fuel hall sensor  2.20-0.94-0.86

      //Shifter.clearRegisters();
      Shifter.setRegisterPin(0, LOW);
      Shifter.setRegisterPin(1, LOW);
      Shifter.setRegisterPin(2, LOW);
      delay(5);
      fuel_cur_value= analogRead(A0) * analog_voltage_part * resistor_divider;

      if (fuel_cur_value != fuel_last_value) {

          for (int i = 1; i <= fuel_max; i++) {
             if ( (fuel_cur_value > FD[i-1]) and (fuel_cur_value <= FD[i]) ) {
              
              fuel_show = ((fuel_cur_value - FD[i-1])  /    // датчик текущее значение минус нижяя граница зоны
                            (FD[i] - FD[i-1]) )        *    // делим на 100% зоны (дельта конца и начала) = процент заполнения зоны
                            (FF[i] - FF[i-1])          +    // умножаем на бензиновый размер зоны
                             FF[i-1];                       // добавляем бензиновое начало зоны
              //     
              break; }}

          if (fuel_cur_value>= FD[0]) {fuel_show=0.0;}
          if (fuel_cur_value< FD[fuel_max]) {fuel_show=FF[fuel_max];}
        
          //TFTprn2(String(fuel_cur_value, 2)+"", 2, 4, 1, ST7735_GREEN, bg_color);
          
          TFTprn2(String(fuel_show, 2)+"", 2, 4, 1, ST7735_GREEN, bg_color);
          
          fuel_last_value = fuel_cur_value; fuel_check_print_timer = millis();  }
      }

//   int           fuel_show = 0;
//   float         fuel_tmp = 0;
//   float FD[] = {  2.2,  2.18,  2.16,  2.14,  2.12,    2.1,   2.08,   2.06,   2.04,   2.02,  1.999,  1.977,  1.952,  1.925,  1.895,  1.863,  1.828,   1.79,  1.680,   1.53,   1.31};
//   float FF[] = {0.000, 2.125, 4.250, 6.375, 8.500, 10.625, 12.750, 14.875, 17.000, 19.125, 21.250, 23.375, 25.500, 27.625, 29.750, 31.875, 34.000, 36.125, 41.225, 46.325, 51.625};
//   int           fuel_max = sizeof(FD)/sizeof(float) - 1; // максимальный индекс FD


void void_gen_voltage_check() {     // green  - 7 - 10.39 - genegator sensor

      Shifter.setRegisterPin(0, HIGH);
      Shifter.setRegisterPin(1, HIGH);
      Shifter.setRegisterPin(2, HIGH);
      delay(5);
      gen_cur_value = analogRead(A0) * analog_voltage_part * resistor_divider;

      if (gen_cur_value != gen_last_value) {
          TFTprn2(String(gen_cur_value, 3)+"v", 2, 4, 107, ST7735_GREEN, bg_color);
          gen_last_value = gen_cur_value; gen_check_print_timer = millis();      }
      }

//ISR for TAH
              void isr_tah_rounds_counter() 
                 {tah_rounds_counter++;}  // счетчик оборотов колеса. обнуляется при снятии показаний из loop

//ISR for SPD
              void isr_spd_counter() {
                cli();
                  tmp_spd_timer = micros();
                  spd_tick_delta_time = tmp_spd_timer - spd_last_time;
                  spd_last_time = tmp_spd_timer;
                  
                  spd_rounds_counter++;                     // счетчик оборотов колеса. обнуляется при снятии показаний из loop
                sei(); }

///////////////////////////////////////////////////////////////
void void_tah_rounds_check() {
          tmp_tah_millis = millis();  
          noInterrupts ();
              tmp_tah_rounds_counter = tah_rounds_counter; tah_rounds_counter=0; // снимаем показания. обнуляем счетчик
          interrupts ();
          
          tah_rpm = (int)(tah_reduction_multiply * tmp_tah_rounds_counter / 
                         (tmp_tah_millis - tah_check_print_timer));
          tah_check_print_timer =  tmp_tah_millis;                     // * 60 * 1000 / 1.73

          tah_show();
          }

///////////////////////////////////////////////////////////////

void void_spd_rounds_check() {
          noInterrupts ();
              tmp_spd_rounds_counter = spd_rounds_counter; spd_rounds_counter = 0; // снимаем показания. обнуляем счетчик
          interrupts ();
                         
          if (tmp_spd_rounds_counter < 30) {  // отсекает показания пройденного пути, если скорость выше 220км/ч
              spd_distance = spd_distance + wheel * tmp_spd_rounds_counter; // счетчик дистанции
              } // проверка на то, что датчик встал напротив колеса
          
          spd_check_print_timer =  millis();
          }
// 200kmh => 100'000 оборотов в час => 27.77 об/сек.          
///////////////////////////////////////////////////////////////

    void void_spd_ticks_detect() {
      noInterrupts ();   // turn interrupts off 
        tmp_spd_tick_delta_time = spd_tick_delta_time;
        tmp_spd_last_time       = spd_last_time;
      interrupts ();
    
     // 35 000 micros() round = 200kmh , ignore less
      
     
      if (old_tick_delta_time != tmp_spd_tick_delta_time or micros() - tmp_spd_last_time > 2500000)
           {
        if ((tmp_spd_tick_delta_time > 2500000) or  (micros() - tmp_spd_last_time > 2500000)) 
             { spd_kmh = 0.0;}
        else { spd_kmh = (float)(wheel * 3600000 / tmp_spd_tick_delta_time); // * 3600 * 1'000'000 / 1'000 km
             }
    
        old_tick_delta_time = tmp_spd_tick_delta_time;
        spd_show();
        }
        }


// TAH SHOW
void tah_show() {
  if (tah_rpm > 6000) {return;}  // cut stupid results: just in case
      if (old_tah_rpm != tah_rpm){
              old_tah_rpm = tah_rpm;
              
              tah_clr = GREEN;
              if (tah_rpm > 2000)  {tah_clr = YELLOW;}
              if (tah_rpm > 2200)  {tah_clr = RED;}

              String xspace="";
              if (tah_rpm < 1000)  {xspace=" ";}
              if (tah_rpm < 100)   {xspace="  ";}
              if (tah_rpm < 10)    {xspace="   ";}
          
          TFTprn2(xspace + String(tah_rpm), 2, 111, 113, tah_clr,  bg_color); //ST7735_BLUE  
          plot_bar(tah_rpm);
          }
}
 
///////////////////////////////////////////////////////////////
               
void spd_show() {
  if (spd_kmh > 220) {return;}
  //if ((float)spd_kmh / old_spd_kmh > 2.0) {return;} // обрезает резкие скачки скорости
  //if ((float)old_spd_kmh / spd_kmh > 2.0) {return;} // 
        if (int(old_spd_kmh) != int(spd_kmh)) {
              old_spd_kmh = spd_kmh;
              spd_clr = GREEN;
              if (spd_kmh >= 75.0)  {spd_clr = YELLOW;}
              if (spd_kmh >= 100.0) {spd_clr = RED;}

              String xspace="";
              if (spd_kmh <100) {xspace=" ";}
              if (spd_kmh <10)  {xspace="  ";}
        
              TFTprn2(xspace + String(int(spd_kmh)), 5, 4, 40, spd_clr, bg_color);  //ST7735_GREEN
        }

  if (spd_distance<5000) { TFTprn2(String(spd_distance/1000, 3)+"", 2, 4, 90, ST7735_GREEN, bg_color); } // show distance
  else {                   TFTprn2(String(spd_distance/1000, 1)+"  ", 2, 4, 90, ST7735_GREEN, bg_color); }
}               

// 4.123
// 999.1**
void plot_bar(int s) {

      if (s>4500) {return;}
      int colorbar = 0;  int cur = 0;
      int start   = 106 - min(s, last_bar) / 45;  // 450 => 10 = 95>
      int finish  = 106 - max(s, last_bar) / 45; //  4500=> 100 = 5

      start  = int(start  / 2) * 2;
      finish = int(finish / 2) * 2;

    for (int16_t y=start; y > finish; y-=2) {
      cur = (106 - y) * 45;

      if (s >= last_bar) {      
          if (cur<2200)  {colorbar = YELLOW;}
          if (cur<1200)  {colorbar = ST7735_GREEN;}
          if (cur>2199)  {colorbar = RED;}
          if (cur>s)     {colorbar = bg_color;}
           }
      else  {colorbar = bg_color; }
      
      tft.drawFastHLine(111, y, 45 , colorbar); }
      last_bar = s;
      }
//

static String ITSPrefix(int ITSPx, int ITSPmaxStringLength) {
    String ITSPspaces = ""; int ITSPcounter = 1;
    while ((ITSPx/=10) > 0) {ITSPcounter++; }
    ITSPmaxStringLength = ITSPmaxStringLength - ITSPcounter;
    if (ITSPmaxStringLength > 0) {
      for (int16_t x=0; x < ITSPmaxStringLength; x++) {ITSPspaces = ITSPspaces + " ";}}
    return ITSPspaces;
}


//
// ==========================================================================================
// ==========================================================================================
// Процедура вывода текста на экран монитора
void TFTprn2(String text, int textsize, int x, int y, int textcolor, int textbg) {
     tft.setTextSize(textsize);           // устанавливаем размер текста
     tft.setTextColor(textcolor, textbg); // устанавливаем цвет текста и фон
     tft.setCursor(x,y);                  // устанавливаем курсор в нужные координаты
     tft.print(text);         }           // печатаем текст через процедуру коррекции русских букв

void TFTprn(String text, int textsize, int x, int y, int textcolor, int textbg) {
     tft.setTextSize(textsize);           // устанавливаем размер текста
     tft.setTextColor(textcolor, textbg); // устанавливаем цвет текста и фон
     tft.setCursor(x,y);                  // устанавливаем курсор в нужные координаты
     tft.print(utf8rus(text));  }          // печатаем текст через процедуру коррекции русских букв

String ITSPrefix2(int ITSPx, int ITSPmaxStringLength) {
    String ITSPspaces = ""; int ITSPcounter = 1; int ITSPshow = ITSPx; bool ITSPminusFlag = false;
    if (ITSPx < 0) { ITSPx = abs(ITSPx); ITSPminusFlag = true;}
    
    while ((ITSPx/=10) > 0) {ITSPcounter++; }
    if (ITSPminusFlag) {ITSPcounter++;}

    ITSPmaxStringLength = ITSPmaxStringLength - ITSPcounter;
    
    if (ITSPmaxStringLength > 0) {
      for (int16_t x=0; x < ITSPmaxStringLength; x++) {ITSPspaces += " ";}}
    return ITSPspaces + String(ITSPshow);
}

String utf8rus(String source) {
 int i,k; String target; unsigned char n; char m[2] = { '0', '\0' }; k = source.length(); i = 0;
 while (i < k) { n=source[i]; i++;
  if (n >= 0xC0) {switch(n) {
                  case 0xD0: { n = source[i];i++;if (n==0x81) {n=0xA8;break;} 
                    if (n>=0x90 && n<=0xBF) n=n+0x2F; break; }
                  case 0xD1: {n=source[i];i++;if (n==0x91) {n=0xB7;break;} 
                    if (n>=0x80 && n<=0x8F) n=n+0x6F;break;}}}
    m[0] = n; target = target + String(m); }
return target; }
// ==========================================================================================
// ==========================================================================================

/* Examples:
void testdrawcircles(uint8_t radius, uint16_t color) {
  //uint16_t time = millis();
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
      //tft.fillCircle(x, y, radius, color); 
    }  } }

tft.setTextColor(); ST7735_BLACK WHITE RED GREEN BLUE MAGENTA CYAN YELLOW
BLACK   0x0000, GRAY    0x8410, WHITE   0xFFFF, RED     0xF800,  ORANGE  0xFA60,
YELLOW  0xFFE0, LIME    0x07FF, GREEN   0x07E0, CYAN    0x07FF, AQUA    0x04FF,
BLUE    0x001F, MAGENTA 0xF81F, PINK    0xF8FF
get resolution: tft.width()  tft.height()           tft.fillScreen(color);                             
tft.setTextWrap(false);                             tft.invertDisplay(true); false
tft.setCursor(0, 0);                                tft.setTextSize(1-6);
tft.print("Hello World!");                          tft.println(8675309, HEX);         
tft.drawRect(x, y, w, h, color);                    tft.fillRect
tft.drawCircle(x, y, radius, color);                tft.fillCircle
tft.drawTriangle(x0, y0, x1, y1, x2, y2, color);    tft.fillTriangle
tft.drawRoundRect(x, y, w, h, radius_ugla, color);  tft.fillRoundRect
tft.drawPixel(x, y, color);                         tft.drawLine(x1, y1, x2, y2, color);
tft.drawFastHLine(0, y, tft.width(),  color);       tft.drawFastVLine(x, 0, tft.height(), color);
char rAr1=16;char lAr1=17;char upAr1=30;char dAr1=31;char wifi=173;char pwrUp=188;char pwrDn=190;char plmin=176;
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define YELLOW  0xFFE0
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define WHITE   0xFFFF
 
Try this. it should convert the standard RGB(255,255,255) to 565 format (FFFF)

word ConvertRGB( byte R, byte G, byte B)
{ return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) ); }
 
 */
// Использую SPI интерфейс устанавливает в пинах 0-1-2 (LOW\HIGH) микросхемы 595 
// 8-битовый код для переключения CD4051 на один из 8 мультиплексорных каналов.
// после идет считывание аналоговых данных именно с этого канала.
// таким образом используя только один вход A0 поочередно можно считывать до 8 датчиков.
// Вход CD4051 обработан делителем напряжения ~ 1/5.6, и может безопасно считывать напряжения
// до 16 вольт (т.е. почти все датчики в машине).
//
//
// float analog_voltage_part = 3.29/1023 ;
// float resistor_divider = 5.6 ;
float ReadAnalogPin(byte pinRead) {
   noInterrupts();
   digitalWrite(latchPin595, LOW);                            // открываем защелку для записи
   shiftOut    (dataPin595, clockPin595, LSBFIRST, pinRead);  // пишем данные  MSBFIRST
   digitalWrite(latchPin595, HIGH);                           // закрываем защелку для записи
   interrupts  ();
   return analogRead(A0) * analog_voltage_part * resistor_divider;
  }


//        pin-code(10.41)
//   brown  - 0 - 10.43 - fuel sensor
// W-orange - 1 - 10.37
//   orange - 2 - 10.39
// W-brown  - 3 - 10.37
//   blue   - 4 - 10.37
// W-green  - 5 - 10.34
// W-blue   - 6 - 10.39
//   green  - 7 - 10.39 - genegator sensor
//
// 
