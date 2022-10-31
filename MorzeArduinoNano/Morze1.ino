//int const Size = 59;
//String letters[Size] = {
//"АаAa=.−",   "БбBb=−...",  "ВвWw=.−−",   "ГгGg=−−.",   "ДдDd=−..",   "ЕеЁёEe=.", 
//"ЖжVv=...−", "ЗзZz=−−..",  "ИиIi=..",    "ЙйJj=.−−−",  "КкKk=−.−",   "ЛлLl=.−..", 
//"МмMm=−−",   "НнNn=−.",    "ОоOo=−−−",   "ПпPp=.−−.",  "РрRr=.−.",   "СсSs=...", 
//"ТтTt=−",    "УуUu=..−",   "ФфFf=..−.",  "ХхHh=....",  "ЦцCc=−.−.",  "Чч=−−−.", 
//"Шш=−−−−",  "ЩщQq=−−.−",  "Ъъ=−−.−−", "ЫыYy=−.−−",  "ЬьXx=−..−",  "Ээ=..−..", 
//"Юю=..−−",   "Яя=.−.−",  
//"1=.−−−−",  "2=..−−−",   "3=...−−",  "4=....−",  "5=.....",    
//"6=−....",   "7=−−...",   "8=−−−..",  "9=−−−−.", "0=−−−−−", 
//",=.−.−.−",  ":=−−−...", ";=−.−.−.", "(){}[]=−.−−.−", "'=.−−−−.", "-=−....−", 
//"/=−..−.",   "?=..−−..",  "!=−−..−−", "|=−...−", "@=.−−.−.", 
//"# ..−.−",        //Конец связи
//"*........",       // Ошибка/перебой
//"''=.−..−.",      // Одинарные кавычки
////'""=.−..−.',    // Двойные кавычки
//" =@",            // Пробел
//".=......"        // Точка
//};

int const Size = 52;
String letters[Size] = {
"Aa=.-",   "Bb=-...",  "Ww=.--",   "Gg=--.",   "Dd=-..",   "Ee=.", 
"Vv=...-", "Zz=--..",  "Ii=..",    "Jj=.---",  "Kk=-.-",   "Ll=.-..", 
"Mm=--",   "Nn=-.",    "Oo=---",   "Pp=.--.",  "Rr=.-.",   "Ss=...", 
"Tt=-",    "Uu=..-",   "Ff=..-.",  "Hh=....",  "Cc=-.-.",  "Qq=--.-",  
"Yy=-.--",  "Xx=-..-",  

"1=.----",  "2=..---",   "3=...--",  "4=....-",  "5=.....",    
"6=-....",   "7=--...",   "8=---..",  "9=----.", "0=-----", 

",=.-.-.-",  ":=---...", ";=-.-.-.", "(){}[]=-.--.-", "'=.----.", "-=-....-", 
"/=-..-.",   "?=..--..",  "!=--..--", "|=-...-", "@=.--.-.", 
"#=..-.-",        //Конец связи
"*=........",       // Ошибка/перебой
"''=.-..-.",      // Одинарные кавычки
" =@",            // Пробел
".=......"        // Точка
};


int dot_duration   = 1000;   // время свечения светодиода при передаче точки
int dash_duration  = 2000;  // время свечения светодиода при передаче тире
int dot_dash_pause = 500;   // время задержки между тире-точка или точка-тире (в одной букве)
int letters_delay  = 300;   // время задержки между буквами
int pause_delay    = 1000;  // время задержки при кодовом знаке @ в кодировке букв
                            //    используется для задержек между словами (пробел, запятая, точка)
int pin = 13;

String MSG = "I want to get 100 point on the exam!  ";

// Sub to transmit dot-dash-pause to the Led
void blink(char cod){ //Serial.println(ID, HEX);
  //Serial.print("> ");
  //Serial.println(cod, HEX);
  switch(cod) {
    case 0x2E: { digitalWrite(pin, HIGH); delay(dot_duration);  break;  }  //"."
    case 0x2D: { digitalWrite(pin, HIGH); delay(dash_duration); break;  }  //"-"
    case 0x40: {                          delay(pause_delay);   break;  }  // "@"
    }
    digitalWrite(pin, LOW); delay(dot_dash_pause); 
  }

int FindSeparPos(String& s){
  //Serial.print("s> "); Serial.println(s);
  for (int idx = 0; idx<s.length(); idx++){
    if (s[idx] == 0x3D) {
      //Serial.print("!> ");
      //Serial.println(idx, HEX);
      return idx+1;}
    }
  
  Serial.println(0, HEX);
  return 0;
  }

void LedOneLetter(int morse_int){
    //Serial.println("=========");
    Serial.println(letters[morse_int]);
    FindSeparPos(letters[morse_int]);
    int start_pos = FindSeparPos(letters[morse_int]);
    for (int i = start_pos; i < letters[morse_int].length(); i++){
      blink(letters[morse_int][i]);
      }
  }

int GetLetterCode(char ch) {
  for (int idx = 0; idx < Size; idx++){
    for (int i = 0; i<letters[idx].length(); i++){
      if (letters[idx][i] == 0x3D) break;             // "=" found - end of search area
      if (letters[idx][i] == ch)   return idx;        // got an index!
      }
    }
  return -1; // Not found
  }

void setup() {
Serial.begin(9600);
pinMode(pin, OUTPUT);
digitalWrite (pin, LOW) ;
}

void loop() {
  for (int idx = 0; idx < MSG.length(); idx++){
    int morse_int = GetLetterCode(MSG[idx]);
    if (morse_int == -1) continue;
    LedOneLetter(morse_int);
    delay(letters_delay);
  }
 }
