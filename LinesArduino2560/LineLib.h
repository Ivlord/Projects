#define Touch_MINPRESSURE       5  // 30 минимальная сила нажатия на экран для срабатывания
#define Touch_Long_Press        750 // время удержания кнопки до срабатывания long_press
#define XP                      6   // настройки калибровки TouchScreen
#define XM                      A2
#define YP                      A1
#define YM                      7
#define TS_LEFT                 949 //945
#define TS_RT                   133 //184
#define TS_TOP                  936 //952
#define TS_BOT                  160 //178

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

constexpr uint16_t pf_db_size         = 128; // размер стационарного буфера для сохранения развития путей
constexpr uint8_t  field_xy_size      = 9;   // размер поля 9x9 шаров. завязано на интерфейс - не изменить
constexpr uint8_t  full_line_size     = 5;   // не менее 5 шаров подряд для удаления линии 
constexpr uint8_t  max_forecast_balls = 3;   // кол-во шаров подсказки =3
constexpr uint8_t  max_balls_colors   = 7;   // кол-во цветов

uint16_t VL[]={0,1,2,318,319, 320, 477,478,479};
uint8_t VL_size = sizeof(VL)/sizeof(uint16_t);
uint16_t HL[]={0,1,2,316,317,318,319};
uint8_t HL_size = sizeof(HL)/sizeof(uint16_t);

class PF {
public:

  MCUFRIEND_kbv tft; //#include <Adafruit_TFTLCD.h> //Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
  TouchScreen touch = TouchScreen(XP, YP, XM, YM,300);
  TSPoint tp;

    uint8_t  fld[field_xy_size][field_xy_size] = {//;   // массив поле 9х9(81)
  // 0     1     2     3     4     5     6     7     8
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 6
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 7
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 8
    };

  uint8_t  selected = 0xff;
  uint16_t score = 0;     // текущий счет игры
  uint8_t pulse = 15;
  unsigned long pulse_update = millis();
  uint8_t xx, yy, cc; // tmp-color
  bool stop_game=false;

  uint8_t bd[pf_db_size];   // база данных, в которой сохраняются развивающиеся пути: 4 бита на направление пути
  uint8_t paths_count = 0;
  uint8_t path_len = 0;
  bool    dir = true;

  uint8_t del_list[8];  // список позиций на поле для удаления
  uint8_t del_cnt = 0;  // количество позиций для удаления
  uint8_t forecast[max_forecast_balls]; // шар -cord
  uint16_t colors[max_balls_colors + 1] = { BLACK, BLUE, RED, GREEN, CYAN, MAGENTA, YELLOW, WHITE };

  uint8_t sx = 0, sy = 0, ex = 0, ey = 0, lx, ly; // start x,y ; end x,y ; last x,y - последние рассчеты
  int8_t model[2][8] = { // up, right-up, right, right-down, down, left-down, left, left-up
               { 0,      1,     1,        1,       0,     -1,      -1,    -1},
               {-1,     -1,     0,        1,       1,      1,       0,    -1} };
  //                         0       1      2         3        4       5        6      7

  void Init(){
    uint16_t ID = tft.readID(); //
    if (ID == 0xD3D3) ID = 0x9481; // write-only shield //   ID = 0x9329;  // force ID
    tft.begin(ID);
    tft.setRotation(1);
    tft.fillScreen(BLACK);
    ResetGame(); //Serial.print(tft.width()); Serial.print("  "); Serial.println(tft.height());    
    }

  void Update(){
    if ((selected!=0xff) and (pulse_update<millis())) { // есть анимация шарика
      if(pulse==15){ pulse=13; 
        for (uint8_t r=15; r>12; r--){ball(selected, BLACK, r, false);             } }
      else{ pulse=15;
        for (uint8_t r=13; r<16; r++){ball(selected, get_clr(selected), r, false); } }
      pulse_update = millis()+300;  
        }
    
    if(Touch_getXY()){ // есть касание  //selected = 0xff
      if(stop_game) {ResetGame(); return;}
      if(tp.x<317){ // поле
        uint8_t _xx = (tp.x-2)/35, _yy=(tp.y-2)/35, _cc=get_clr(_xx,_yy);   // cc-color index
        //if (!_cc and (selected==0xff)) {return;}
        if (_cc){  // поле в месте клика имеет цвет=стоит какой-то шар      // select new ball
          if (selected!=0xff){ball(selected, get_clr(selected), 15, true);} // восстановить предыдущий выбор
          selected=xy2cord(_xx, _yy);                                       // новый выбор
          pulse_update = millis();
          }
        else if(selected!=0xff){// если кликнули по пустой клетке и есть выбранный шар. самое сложное!!
          // делаем проверку, возможен ли перекат
          cord2xy(selected, sx, sy);
          uint8_t res = pf_all(sx, sy, _xx, _yy, 0, 0b10101010);
          if (res!=0xff) { // путь найден
            //Serial.print(" path FOUND:");Serial.print(int(selected),HEX);Serial.print("->");Serial.println(int(xy2cord(_xx, _yy)),HEX);
            //prn(res);
          uint8_t x_cord = sx, y_cord = sy; int16_t ptr = res * path_len; // отрисовываем путь
          for (uint8_t i = 0; i < path_len; i++, ptr++) { 
            uint8_t _block = ReadBlock(ptr, dir); x_cord += model[0][_block]; y_cord += model[1][_block];
            ball(xy2cord(x_cord, y_cord), get_clr(selected), 8, true);
          }
          delay(200);                                                     // задержка, чтобы пользователь успел увидеть путь
          x_cord = sx; y_cord = sy; ptr = res * path_len;                 // стираем путь
          for (uint8_t i = 0; i < path_len; i++, ptr++) {
            uint8_t _block = ReadBlock(ptr, dir); x_cord += model[0][_block]; y_cord += model[1][_block];
            ball(xy2cord(x_cord, y_cord), 0, 8, true);
          }

            ball(xy2cord(_xx, _yy), get_clr(selected), 15, true); // переставляем шар на новое место
            ball(selected, 0, 15, true);                          // стираем старый шар
            selected=0xff;                                        // распомечаем выбор

            // проверяем не получилась ли линия
            if (!CheckFullLines(_xx, _yy)) {PullForecast();} // если ни одного шара не убралось, докидываем новые 3 шара
            }
          else{// не найден: ничего не делаем
            //Serial.print(" path not found:");Serial.print(int(selected),HEX);Serial.print("->");Serial.println(int(xy2cord(_xx, _yy)),HEX);
            }
          }
        
        //tft.drawPixel(tp.x, tp.y, YELLOW);//Serial.print("\n field:");Serial.print(int(_xx));Serial.print(", ");Serial.print(int(_yy));Serial.print(", clr="); Serial.println( int(_cc));Serial.print("    ");Serial.print(tp.x);Serial.print(", ");Serial.println(tp.y);
        }
      else if(tp.y<200){ // pull new
        PullForecast();        
        }
      else if(tp.y<258){ // restart
        ResetGame(); 
        }
      else { // options
        Serial.println("\n\noptions button redraw");
        }

      
    }//end_Touch_getXY()
  }

  void ResetGame(uint16_t _score = 0) {
    stop_game=false;
    ScanFreeCells(true); // just_clear=true -> очищаем всё поле
    score = _score;  
    selected = 0xff;
    pulse = 15;
    DrawField();
    for(uint8_t i=0;i<max_forecast_balls;i++){forecast[i]=0xff;} // clear forecast
    PullForecast(); PullForecast();    
    }

  void ball(uint8_t _cord, uint8_t col, uint8_t _r=15, bool _fill=true) { // отрисовка и удаление шаров, изменение базы
    // !! изменение базы происходит ТОЛЬКО, если r==15 и fill==true
    // мелкие шары пути и пульсации выбранного шара не изменяют базу,
    uint8_t _x,_y; cord2xy(_cord, _x, _y);
    //Serial.print("\n    ball:"); Serial.print(int(_cord), HEX); Serial.print(", r="); Serial.print(int(_r)); Serial.print(", fill="); Serial.print(_fill); Serial.print(", col="); Serial.println(col);
    if ((_x==0x0f) or (col>max_balls_colors)) return;                       // ошибочные координаты. используется. не удалять!
    if (_y==0x0f){tft.fillCircle(uint16_t(358+_x*40),175,_r,colors[col]);
      //Serial.println("     fore");
      }  // маркер того, что шар в форекастах _x=[0,1,2]
    else{
      if(_fill) {                                                           // заполняем или только отрисовываем контур
            if (_r==15) {fld[_y][_x] = col&0x0f; 
            //Serial.println("     bd change"); 
            }
            //Serial.println("     field #");
            tft.fillCircle(uint16_t(20+_x*35), 20+_y*35, _r, colors[col]);
            } // изменения в базе, если r=15 и fill
      else  
      {tft.drawCircle(uint16_t(20+_x*35), 20+_y*35, _r, colors[col]);
      //Serial.println("     pulse");
      };  // просто пульсация шара. нет изменений в базе
    }
  }
  uint8_t PullForecast() {                // выставляет шары из forcast и генерирует следующую подсказку
    //Serial.print("\n> PullForecast:");
    for (uint8_t i=0;i<max_forecast_balls;i++) {
      ScanFreeCells();                    // paths_count=кол-во свободных клеток в формате cord
      //Serial.print(" free cells:"); Serial.println(paths_count); 
      if (!paths_count) {stop_game = true; TFTprn( "LOSE!", 10, 10, 120, MAGENTA, 0);
        return 0;}
      uint8_t _xxx, _yyy, new_cord = bd[ rand()%paths_count ]; cord2xy(new_cord, _xxx, _yyy);
      ball(new_cord, forecast[i], 15, true);// добавляем шар из форекаста
      CheckFullLines(_xxx, _yyy);
      if(stop_game) return 0;
      
      forecast[i] = (rand() % max_balls_colors) + 1;                  // шар нового цвета на место только что выставленного
      ball( xy2cord(i,0x0f), forecast[i], 15, true);        // 
    }
      ScanFreeCells();                    // paths_count=кол-во свободных клеток в формате cord
      if (!paths_count) {stop_game = true; TFTprn( "LOSE!", 10, 10, 120, MAGENTA, 0);
        return 0;}    
  }
  void ScanFreeCells(bool just_clear = false) { // заносит в bd все cord найденных пустых клеток. количество будет в = paths_count
    paths_count = 0;
    for (int y = 0; y < field_xy_size; y++) {
      for (int x = 0; x < field_xy_size; x++) {
        if (just_clear) { fld[y][x] = 0; continue; }
        if (get_clr(x, y)) continue;                  // если клетка занята, пропускаем её
        bd[paths_count++] = xy2cord(x, y);            // добавляем в bd найденное пустое поле
      }}}
  uint8_t CheckFullLines(uint8_t x, uint8_t y) {
    // проверяет на поле, образовались ли полные линии длиной full_line_size. Линии должны проходить
    // через (x,y). Обслуживается до 33 удалений (все варианты) в случае, если проверяемый
    // шарик дополнил все линии (мульти удаление полных линий)
    uint8_t clr = get_clr(x, y);
    if (!clr) {return 0;}
    uint8_t total_deleted = 0;
    if (!clr) return 0xff;      //?? цвет==0 - нет шарика
    bool clear_main_ball = false;
    del_cnt = 0;
    for(uint8_t line_model = 0; line_model<4; line_model++){ // 4-линии: верт, гориз, 2 диагон
      uint8_t cur_model = line_model;
      for (uint8_t rays = 0; rays < 2; rays++) {  // два противоположных луча в линии
        uint8_t nx=x, ny=y;
        for (uint8_t cells = 0; cells < 9; cells++) {
          nx += model[0][cur_model]; ny += model[1][cur_model];
          if (!in_range(nx,ny) or (clr != get_clr(nx, ny)) ) break;
          del_list[del_cnt++] = xy2cord(nx, ny);
        }
        cur_model += 4; // model: 0->4, 1->5, 2->6, 3->7
      }
      if (del_cnt < full_line_size-1) {
        del_cnt = 0;// ничего не нашли. обнуляем буфер
        continue; } // если количество найденных шариков в обеих лучах <4 (<5 с проверяемым)
      clear_main_ball = true;   // помечаем, что и проверяемый шар надо будет убрать
      total_deleted += del_cnt;
      for (;del_cnt;) {     // пока del_cnt>0
        ball(del_list[--del_cnt], colors[0], 15, true); // удаления шарика с экрана
      }
    }
    if(clear_main_ball){ 
      ball(xy2cord(x, y), colors[0], 15, true); // удаляем главный (проверяемый) шарик
    total_deleted++;} 

    if(total_deleted){                       // пересчитываем и обновляем счет
      score+=total_deleted*20; 
      if (total_deleted>full_line_size){score+=(full_line_size-total_deleted)*10;}          // бонус за дополнительные шары
      if(uint16_t(score/10)>999) {score=9990; stop_game = true; TFTprn( "WIN!", 10, 10, 120, GREEN, 0);}
      TFTprn( SSA(String(uint16_t(score/10)), 3), 7, 340, 95, GREEN, 0);// выводим счет
      }
    
    return total_deleted; // возвращаем общее количество удаленных шариков
  }
  uint8_t ReadBlock(uint16_t block, bool direction) {
    // читаем блок данных по 4 бита с начала (direction = true) или конца(direction = false)
    if (block >= (pf_db_size<<2) ) {return 0x0f;}
    if (!direction) block = 2 * pf_db_size - block - 1;
    if (block & 0x01) return bd[block >> 1] & 0x0f; // x 1
    return bd[block >> 1] >> 4; // >>0
  }
  void WriteBlock(uint8_t data, uint16_t block, bool direction) {
    if (block >= (pf_db_size << 2)) { return; }
    if (!direction) block = 2 * pf_db_size - block - 1;
    int16_t ptr = block >> 1;
    if (block & 0x01) { bd[ptr] = (bd[ptr] & 0xf0) | (data & 0x0f); }// если нечёт: обнуляем мл.биты байта в базе и or-ом накладываем младшие
    else {        bd[ptr] = (bd[ptr] & 0x0f) | (data << 4  ); }// если чет: обнуляем ст. биты
  }
  uint8_t CopyPath(uint8_t src, uint8_t dst, uint8_t new_point = 0x0f) {
    // src - номер пути откуда, dst - Номер пути куда
    // копирует из текущей базы (направление dir) путь в инвертное направление (!dir path_len+1)
    // возвращает № block куда занести расширение пути
    // если new_point!=0x0f, записывает расширение в блок номер которого возвращается в dptr
    if (src >= paths_count) return 0xff;
    int16_t sptr = src * path_len;    // указатель на начало пути src
    int16_t dptr = dst * (path_len + 1);  // указатель на начало пути dst
    for (; sptr < (src + 1) * path_len; sptr++, dptr++) {
      WriteBlock(ReadBlock(sptr, dir), dptr, !dir); }
    if (new_point != 0x0f) { 
      WriteBlock(new_point, dptr, !dir); }
    return dptr;
  }
  void cord2xy(uint8_t _cord, uint8_t& _x, uint8_t& _y) {_y=_cord&0x0f; _x=_cord>>4;}
  uint8_t xy2cord(uint8_t x, uint8_t y) { return ((x<<4) + (y&0x0F)); }
  void path_last(uint8_t _path) { // просчитывает конечную точку пути _path. Помещает значения в (lx, ly)
  // берет текущий dir, path_len, sx,sy - начало пути, проверяет чтоб номер пути не был больше paths_count
    if (_path >= paths_count) { lx = ly = 0xff; return; }
    lx = sx; ly = sy;
    uint16_t ptrS = _path * path_len;
    for (int i = 0; i < path_len; i++, ptrS++) {
      uint8_t tmp = ReadBlock(ptrS, dir);
      lx += model[0][tmp]; ly += model[1][tmp];
    }
  }
  bool in_range(uint8_t x, uint8_t y) { return ((x >= field_xy_size) or (y >= field_xy_size)) ? false : true; }
  uint8_t get_clr(uint8_t x, uint8_t y) {
    return ((in_range(x, y)) ? (fld[y][x] & 0x0f) : 0xff);} // ошибочный диапазон
  uint8_t get_clr(uint8_t _cord){ cord2xy(_cord,xx,yy); 
    return ((in_range(xx,yy)) ? (fld[yy][xx]&0x0f):0xff); }
  void cell_busy_set(uint8_t _x, uint8_t _y) { fld[_y][_x] = fld[_y][_x] | 0x80; }
  void cell_busy_clear(uint8_t _x, uint8_t _y) { fld[_y][_x] = fld[_y][_x] & 0x7f; }
  bool cell_busy(uint8_t _x, uint8_t _y) { return bool(fld[_y][_x] & 0x80); }
  bool cell_ok(uint8_t _x, uint8_t _y, uint8_t color) { // клетка подходит: нет маркера busy и inFieldRange
    return (!in_range(_x, _y)) ? false:(!cell_busy(_x, _y) and (get_clr(_x, _y) == color));}
  uint8_t pf_all(const uint8_t _sx, const uint8_t _sy, const uint8_t _ex, const uint8_t _ey, const uint8_t _clr = 0, const uint8_t mod = 0b10101010) {
    // 0ff- если путь не найден, или найденного пути номер пути.
    uint8_t step = 0; uint8_t res = 0xff;
    dir = true; path_len = 0; paths_count = 1;
    sx = _sx; sy = _sy; ex = _ex; ey = _ey;
    for (int y = 0; y < field_xy_size; y++) { for (int x = 0; x < field_xy_size; x++) { cell_busy_clear(x, y); } } // сбрасываем бит маркера занятости по всем клеткам поля
    cell_busy_set(sx, sy);
    while (paths_count and (res == 0xff)) { res = pf_onestep(_clr, mod); step++; }
    return res;
  }
  uint8_t pf_onestep(uint8_t _clr = 0, uint8_t mod = 0b10101010) { // sx, sy, ex, ey уже должны быть установлены. 
    // dir сбрасывается в true, 
    // paths_count=обнуляется перед стартом при выходе содержит номер пути с ответом, если нашли (true)
    // path_len=обнуляется и при выходе содержит длину пути по шагам
    // содержит только ШАГИ, т.е. дойти до конца можно по шагам
    uint8_t _mod;
    uint8_t new_paths = 0;  // количество обнаруженныя новых путей
    uint8_t nx, ny;     // New coords
    uint8_t res = 0xff;

    //for (uint8_t _path = 0; _path < paths_count; _path++) { // пошли по путям в базе
    for (int _path = paths_count - 1; _path >= 0; _path--) { // пошли по путям в базе
      path_last(uint8_t(_path)); // находим lx, ly
      _mod = mod;

      for (uint8_t _bit = 0; _bit < 8; _bit++) {
        nx = lx + model[0][_bit]; ny = ly + model[1][_bit];
        if ((_mod & 0x80) and cell_ok(nx, ny, _clr)) { // координата in range
          CopyPath(uint8_t(_path), new_paths, _bit);
          cell_busy_set(nx, ny);
          new_paths++;
          if ((nx == ex) and (ny == ey)) { res = new_paths - 1; break; }
        }
        _mod = _mod << 1;
      }
      if (res != 0xff) {break;}
    }
    dir = !dir;
    path_len++;
    paths_count = new_paths;
    return res;   // возвращаем или 0xff, если ничего не найдено или номер найденного пути: this.prn(res)
    // в случае измерения линии при заполнении вернет 0xff и забрать надо при помощи:
    // 1) путь под номером 0, 2) длина пути вычесть 1, 3) not (dir) или => this.prn(0, true);
  }



  void prn(uint8_t _path, bool reverse = false) {// рапечатывает путь с учетом настроек
    // распечатывает путь. если reverse = false, длина=path_len, иначе считается, что это более
    // длинный путь "из прошлого" (длина=path_len-1)
    uint8_t p_len = (reverse) ? (path_len - 1) : path_len;
    bool  p_dir = (reverse) ? (!dir) : dir;
    uint8_t x_cord = sx, y_cord = sy;

    Serial.print("(");Serial.print(int(x_cord));Serial.print(int(y_cord));Serial.println(") >");
    int16_t ptr = _path * p_len;
    for (uint8_t i = 0; i < p_len; i++, ptr++) {
      uint8_t _block = ReadBlock(ptr, p_dir);
      x_cord += model[0][_block]; y_cord += model[1][_block];
      Serial.print(int(_block));Serial.print("(");Serial.print(int(x_cord));Serial.print(int(y_cord));Serial.print(") - ");
    }
    Serial.println("");
  }
  
  void DrawField(){
    tft.fillScreen(BLACK);
    for(uint8_t i=0; i<VL_size; i++){tft.drawFastVLine(VL[i], 0, 320, RED);}
    for(uint8_t i=0; i<8; i++){tft.drawFastVLine(37+i*35, 0, 320, RED);}
    
    for(uint8_t i=0; i<HL_size; i++){tft.drawFastHLine(2, HL[i], 476, RED);}
    for(uint8_t i=0; i<8; i++){tft.drawFastHLine(2, 37+i*35, 316, RED);}
  
    //for(uint8_t y=0; y<9; y++){
    //for(uint8_t x=0; x<9; x++){
    //  tft.fillCircle(20+x*35, 20+y*35, 15, rand()%0xffff);
    //  }}
    //for(uint8_t x=0; x<3; x++){
    //  tft.fillCircle(358+x*40, 175, 15, rand()%0xffff);
    //  }
  
    TFTprn("By IvLord (c)", 1, 398, 7, YELLOW, 0); // 400, 5
    TFTprn("Lines", 4, 340, 25, YELLOW, 0);
    TFTprn("Score:",   2, 340, 70, GREEN, 0); // 340, 80
    TFTprn(SSA(String(score),3), 7, 340, 95, GREEN, 0);
  
    tft.fillRect(326, 205, 146, 50, BLUE);
    TFTprn("Restart",   3, 335, 218, GREEN, BLUE);
    tft.drawRect(327, 206, 144, 48, WHITE);
    
    tft.fillRect(326, 260, 146, 50, BLUE);
    TFTprn("Options",   3, 335, 273, GREEN, BLUE);
    tft.drawRect(327, 261, 144, 48, WHITE);
    }

  bool Touch_getXY() {
      //TSPoint p;
      tp = touch.getPoint();
      pinMode(YP, OUTPUT); digitalWrite(YP, HIGH);        // restore shared pins
      pinMode(XM, OUTPUT); digitalWrite(XM, HIGH);        // because TFT control pins
      if (tp.z < Touch_MINPRESSURE) { tp.z=0; }
      else {
        uint16_t _tmp = map(tp.y,TS_LEFT,TS_RT,0,480); //w
        tp.y = map(tp.x,TS_BOT,TS_TOP,0,320); // h
        tp.x=_tmp+5;} // !!+5 correction
      return tp.z!=0; } //else {word _tmp = map(p.y,949,133,0,480); p.y = map(p.x,936,160,0,320); p.x=_tmp; }

// ==========================================================================================
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
String SSA(String txt, uint8_t MaxLenNeed, char ch=' '){ // увеличивает длину String лидирующими пробелами
  int8_t aLen = MaxLenNeed - txt.length();
  if (aLen>0){ for (int8_t i=0; i<aLen; i++){txt = ch+txt;}}
  return txt;
  }
// ==========================================================================================

};
