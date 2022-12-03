
#include <EEPROM.h>
// Подключение PZEM004 ========================================================================
#include <PZEM004T.h>           // Включаем стандартную библиотеку для работы с модулем PZEM004
HardwareSerial hwserial(UART0); // Используем аппаратный UART0 (GPIO1-TX и GPIO3-RX)
PZEM004T pzem(&hwserial);       // Подключаем PZEM к UART0
IPAddress ip(192,168,1,2);      // Назначаем ему отдельный IP-адрес
//
// Подключение SPI монитора ===================================================================
#include <SPI.h>                // Стандартная библиотека для работы с SPI-устройствами
#include <Adafruit_GFX.h>       // Общая графическая библиотека для рисования на мониторах
#include <TFT_ILI9163C.h>       // Библиотека для работы непосредственно с нашим монитором 
                                // (чип ILI9163C)
#define __CS  5                 // GPIO5-D1 => Cable Select для монитора
#define __DC  2                 // GPIO2-D4 => RegisterSelect для монитора
#define __RES 255               // означает, что подключен к RST-контакту NodeMCU-ESP8266
TFT_ILI9163C tft = TFT_ILI9163C(__CS, __DC, __RES); // подключаем монитор, присваиваем ему 
                                // имя tft 
//
//                             Для удобства использования назначаем некоторым цветам названия
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
//                                 Для удобства использования назначаем символы псевдографики
char rAr1=16;  char lAr1=17;  char upAr1=30; char dAr1=31;  
char wifi=173; char pwrUp=188; char pwrDn=190; char plmin=176;
// ==========================================================================================

#define KOTEL 0           // GPIO0-D3 - Контакт управления питанием котела
#define TEMPERATURA 4     // GPIO4-D2 - Контакт подключенного датчика температуры

//
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(TEMPERATURA);        // Setup a oneWire instance to communicate with any 
                                     // OneWire devices
DallasTemperature sensors(&oneWire); // Передаём библиотеке ссылку на подключенный через 
                                     // OneWire датчик температуры
DeviceAddress insideThermometer;     // arrays to hold device address
// ==========================================================================================
//
bool PWR_ON = false ; // True-котел включен, false-котел выключен
int keyvalue = 0; bool pzemrdy = false; 

int EEPROM_address = 100;
float targetTemp=28.00; 
float gisterezis = 1.0;
float tempC=0.0;
float v=0.0;
float a=0.0;
float p=0.0;
float e=0.0;
String tmpp="123";
int keymode=0; // 0-2
int setting=0; // 0-1
float addsetting=0.0; //
int setting_colors[] = {CYAN, CYAN};
unsigned long setuptime=millis();
unsigned long starton=millis();
unsigned long startoff=millis()+1;
unsigned long worktimetotal=0;
float Ztek = 0.0;
float Zsr = 0.0;
unsigned long timeoff=0;
unsigned long timeon=0;

// Подключение WiFi =========================================================================

#include <ESP8266WiFi.h>        // Стандартная библиотека поддержки WiFi на модуле ESP8266
#include <WiFiClient.h>         // Стандартная библиотека клиент-сервера
#include <ESP8266WebServer.h>   // Стандартная библиотека Web-сервера для модуля ESP8266
//
const char *ssid = "ESPap";     // Определяем название нашей WiFi сети (SSID)
const char *password = "1234567890"; // Определяем пароль для подключения к нашей сети
ESP8266WebServer server(80);    // запускаем Web-сервер
// HTML текст, который будет посылать наш WIFI-сервер при подключении к нему
// параметр PROGMEM означает, что этот текст загружен в FLASH-память ESP8266
// он не будет занимать оперативную память и его нельзя изменять из программы
//
const char html_header[] PROGMEM =
" <!DOCTYPE html><html><head><meta charset='utf-8'><title>Система управления котлом</title>"
"<style>"
"* {font-family: 'Times New Roman', Times, serif;}"
"h1 {font-size: 40pt;color: #00008B;}"
"p {font-size: 35pt;}"
".c {border: 1px solid #333;"  /* Рамка */
"display: inline-block; padding: 25px 45px;" /* Поля */
"text-decoration: none;" /* Убираем подчёркивание */
"color: #a00;bgcolor='red';font-size: 35pt;}" /* Цвет текста */
"</style></head>";
// ==========================================================================================


void setup() { 
  tft.begin(); tft.clearScreen(1); tft.setRotation(3); 
  pinMode (KOTEL, OUTPUT);    // переводим контакт, к которому подключено управление котлом 
                              //  в режим Вывода
  digitalWrite(KOTEL, LOW);   // отключаем котел - подаем на контакт LOW=GND=0V
// ==========================================================================================

  TFTprn("Запуск",       3, 13,  10, BLUE,   1);
  TFTprn("системы",      2, 25,  34, BLUE,   1);
  TFTprn("котла.",       2, 33,  50, BLUE,   1);
  TFTprn("Мазлов Иван",  1, 25, 110, YELLOW, 1);

  sensors.begin(); 
  if (!sensors.getAddress(insideThermometer, 0)) TFTprn("Нет датчика t !!",1,3,68, RED, 1);
  else TFTprn("Датчик t #"+String(sensors.getDeviceCount())+" адр:"+String(insideThermometer[0], HEX)+"h", 1, 3, 68, YELLOW, 1);


  // ==========================================================================================
  sensors.setResolution(insideThermometer, 10);
  //delay(2000);

  if (sensors.isParasitePowerMode()) TFTprn("Питание: пар",  1, 3, 78, GREEN, 1); 
  else                               TFTprn("Питание: норм", 1, 3, 78, GREEN, 1);
  TFTprn("Точность бит: "+String(sensors.getResolution(insideThermometer)),1,3,88,GREEN,1);
  //delay(10000);

  tft.clearScreen(1);
  TFTprn(String(wifi), 2, 116, 5, RED, 1); // значек WiFi

  for (int i = 5; i>-1; i--) {
      TFTprn("Ждем модуль pzem:" + String(i), 1, 3, 30, GREEN, 0);
      pzemrdy = pzem.setAddress(ip);
      //delay(1000);
      if (pzemrdy) break;
      }
// ==========================================================================================
  TFTprn("Включаем WiFi...", 1, 3, 40, GREEN, 0);
  WiFi.softAP(ssid, password, 8); /* remove the password if you want the AP to be open.*/
  IPAddress myIP = WiFi.softAPIP();
  server.on("/", handleRoot);
  server.begin();

  TFTprn("ssid: " + String(ssid),     1, 3, 48, GREEN, 0);
  TFTprn("pass: " + String(password), 1, 3, 56, GREEN, 0);
  TFTprn("ip: 192.168.4.1", 1, 3, 64, GREEN, 0);            // String(myIP)
  TFTprn(String(wifi), 2, 116, 5, GREEN, 1); // значек WiFi
  //delay(7000);

  tft.clearScreen(1);
  
  TFTprn(String(wifi), 2, 116, 5, GREEN, 1); // значек WiFi
  TFTprn("O", 2, 64, 5, GREEN, 1);           // значек градусов
  TFTprn("A", 2, 51, 55, BLUE, 1);           // значек амперов

  EEPROM.begin(512);
  
  targetTemp = EEPROM.read(EEPROM_address)  * 0.5; 
  gisterezis = EEPROM.read(EEPROM_address+1)* 0.5;


  
  setuptime=millis();
}

// ==========================================================================================

void loop(void) {

  sensors.requestTemperatures(); // Send the command to get temperatures
  tempC = sensors.getTempC(insideThermometer);

  TFTprn(String(int(tempC)), 5, 3, 5, GREEN, 1);
  tmpp=String(tempC-int(tempC),1); tmpp.remove(0, 1); TFTprn(tmpp, 2, 60, 29, GREEN, 1);
  
  if (!pzemrdy) {
      pzemrdy = pzem.setAddress(ip);  // если pzem не был сразу найден, подключаем снова
      v=230.35; a=28.66; p=6230.35; e=286230.35; }
  else {
      v = pzem.voltage(ip);   // текущее напряжение сети, V
      a = pzem.current(ip);   // текущая сила тока нагрузки, A
      p = pzem.power(ip);     // рассчитанная модулем мощность потребления, W
      e = pzem.energy(ip);    // рассчитанное потребление за всё время работы модуля, W
      }

  tmpp=String(a-int(a),1);
  tmpp.remove(0, 1);
  if (a<10) { TFTprn(String(int(a)), 4, 27, 55, BLUE, 1);} 
  else {      TFTprn(String(int(a)), 4,  3, 55, BLUE, 1);}
  TFTprn(          tmpp, 2, 50,              71, BLUE, 1); //десятичное значение
  
  TFTprn(String(v,1)+      "V "  , 1, 77, 55, GREEN, 1);   // напряжение
  TFTprn(String(p,0)+      "W  "  , 1, 77, 65, GREEN, 1);  // мощность
  TFTprn(String(e/1000,0)+ "kWh ", 1, 77, 75, GREEN, 1);   // потребление

  TFTprn("Загр: " + String(Zsr,0) + "%, Ср:" + String(Ztek,0) + "%", 1, 3, 90, GREEN, 1); // потребление

  keyvalue=analogRead(A0) >> 8;
  TFTprn(String(keyvalue), 2, 92, 5, CYAN, 1); // нажатая кнопка

  if (keyvalue==1) {
    keymode++; 
    if (keymode>2) {
      keymode=0;
      if(EEPROM.read(EEPROM_address)   != byte(targetTemp/0.5)) {
        EEPROM.write(EEPROM_address   , byte(targetTemp/0.5));
        EEPROM.commit();
        }
      if(EEPROM.read(EEPROM_address+1) != byte(gisterezis/0.5)) 
      {EEPROM.write(EEPROM_address+1 , byte(gisterezis/0.5));
       EEPROM.commit();
         }
      }
  }
  
  if (keymode==1 and keyvalue>1) {setting++; if (setting>1) setting=0;}
  
  addsetting=0.0;
  if (keymode==2 and keyvalue==2) addsetting=0.5;
  if (keymode==2 and keyvalue==4) addsetting=-0.5;

  if (setting==0) targetTemp=targetTemp+addsetting;
  if (setting==1) gisterezis=gisterezis+addsetting;
  if (gisterezis<0.5) gisterezis=0.5;
  if (targetTemp<15.0) targetTemp=15.0;

  
  setting_colors[0]=CYAN; 
  setting_colors[1]=CYAN;
  
  if (keymode==1) setting_colors[setting]=YELLOW;
  if (keymode==2) setting_colors[setting]=RED;

// ==========================================================================================

  TFTprn(String(targetTemp,1)+" C", 1, 92, 22, setting_colors[0], 1);              // уст. t
  TFTprn("("+String(plmin)+String(gisterezis,1)+")", 1, 92, 32, setting_colors[1], 1); // гис



  //TFTprn(" : " + String(millis()), 1, 50, 110, GREEN, 1);
  if (PWR_ON and tempC>targetTemp+gisterezis) {  // если включено и t>Нормы - выключаем котел
    // выводим значек pwrDn
    PWR_ON=false;  digitalWrite(KOTEL, LOW); TFTprn(String(pwrDn), 2, 104, 5, BLUE, 1);
    timeon=millis()-starton;
    //Ztek=(float)(timeon/(timeon+timeoff));
    Ztek=(float)timeon+timeoff;
    Ztek=(float)timeon/Ztek;
    Ztek=(float)Ztek*100;

    worktimetotal=worktimetotal+timeon;
    //Zsr=(float)(worktimetotal/(millis()-setuptime));
    Zsr=(float) millis()-setuptime;
    Zsr=(float) worktimetotal/Zsr;
    Zsr=(float)Zsr*100;
    
    startoff=millis();      

    //TFTprn("+ON: " + String(timeon) + " off:" + String(timeoff), 1, 3, 100, GREEN, 1);
    //TFTprn("wtt: " + String(worktimetotal), 1, 3, 110, GREEN, 1);
    
    } 
    
  if (!PWR_ON and tempC<targetTemp-gisterezis) { // если включено и T<Нормы - включаем котел
    // выводим значек pwrDn
    PWR_ON=true; digitalWrite(KOTEL, HIGH); TFTprn(String(pwrUp), 2, 104, 5, RED, 1); 
    timeoff=millis()-startoff;
    Ztek=(float)timeon+timeoff;
    Ztek=(float)timeon/Ztek;
    Ztek=(float)Ztek*100;
    
    Zsr=(float) millis()-setuptime;
    Zsr=(float) worktimetotal/Zsr;
    Zsr=(float)Zsr*100;
    
    starton=millis();
    //TFTprn("+ON: " + String(timeon) + " off:" + String(timeoff), 1, 3, 100, GREEN, 1);
    } 

  server.handleClient();
}

void handleRoot() {

  String message = "<body><h1>Система управления котлом</h1><br>";
  
  message += "<p>";                   //Add a new line
  message += "Температура:     "+String(tempC)+"C<br>";
  message += "Напряжение сети: "+String(v)+"V<br>";
  message += "Ток нагрузки:    "+String(a)+"A<br>";
  message += "Мощность:        "+String(p)+"W<br>";
  message += "Расход Итого:    "+String(e)+"Wh<br>";
  message += "Средняя загрузка:    "+String(Zsr,1)+"%<br>";
  message += "Текущая загрузка:    "+String(Ztek,1)+"%<br>";

for (int i = 0; i < server.args(); i++) {
  if (server.arg("addgist")!= ""){
    if (server.arg(i)=="1") 
    {gisterezis=gisterezis+0.5;} 
    else {gisterezis=gisterezis-0.5;}
      if(EEPROM.read(EEPROM_address+1) != byte(gisterezis/0.5)) 
      {EEPROM.write(EEPROM_address+1 , byte(gisterezis/0.5));
       EEPROM.commit();}
    }
    
  if (server.arg("addt")!= ""){
    if (server.arg(i)=="1") 
    {targetTemp=targetTemp+0.5;} 
    else {targetTemp=targetTemp-0.5;}
      if(EEPROM.read(EEPROM_address)   != byte(targetTemp/0.5)) {
      EEPROM.write(EEPROM_address   , byte(targetTemp/0.5));
      EEPROM.commit();}
    }
    } 
  if (gisterezis<0.5) gisterezis=0.5;
  if (targetTemp<15.0) targetTemp=15.0;

  message += "<br>Заданная t= "+ String(targetTemp, 1)+"   ";
  message += "<a href='?addt=1' class='c'>    +    </a>"; //http://
  message += "<a href='?addt=-1' class='c'>    -    </a>";
  
  message += "<br>Гистерезис= "+ String(gisterezis, 1)+"   ";
  message += "<a href='?addgist=1' class='c'>    +    </a>"; //http://
  message += "<a href='?addgist=-1' class='c'>    -    </a>";
  
  server.send(200, "text/html", html_header+message+"</p></body></html>");    //text/plain
}


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


