#include "CfgParam.h"
#include "Buttons.h"
#include <EEPROM.h>
#include <LiquidCrystal.h> // LCD 16*2
extern LiquidCrystal lcd; //Глоб перемен определенные в другом файле
extern volatile boolean blink500ms;
extern volatile byte MenuTimeoutTimer; //Глобальная переменная определенная в другом файле


//template <class T> void sort()
CfgParamF::CfgParamF():
  _val(0),
  _min(0),
  _max(0),
  _step(0),
  _show_plus(false),
  _icon(0),
  _leftesc(true)
{
}

CfgParamF::~CfgParamF()
{
}



CfgParamF::CfgParamF(const float v, float min, float max, float step):
  _val(v), _min(min),
  _max(max),
  _step(step),
  _show_plus(false),
  _icon(2),//По умолчанию печатаь значок градуса.
  _leftesc(true),
  _x(0),
  _y(1)

{
}


float CfgParamF::val()
{
  return _val;
}
float CfgParamF::minval()
{
  return _min;
}

float CfgParamF::maxval()
{
  return _max;
}

float  CfgParamF::step()
{
  return _step;
}


//////////////////////////////

float CfgParamF::val( const float v)
{

  return  _val = v;
}

float CfgParamF::minval( const float v)
{

  return  _min = v;
}

float  CfgParamF::maxval(const float v)
{

  return _max = v;
}

float CfgParamF::step(const float v)
{
  return _step = v;
}

boolean CfgParamF::show_plus(boolean t)
{
  return _show_plus = t;
}

boolean CfgParamF::leftesc(boolean t)
{
  return _leftesc = t;
}
//Функция для изменения значка величины которая выводится в функциии edit . Если 0  - то не будет печатаься
char CfgParamF::icon(char t)
{
  return _icon = t;
}

//////////////////////////////////////////////////////////////////////
int CfgParamF::edit()
{
  float tmp = _val;
  int key = btnNONE;
  char buff[17];
  char str_buf[17];

  do {
    dtostrf(_val, 2, 1, str_buf);
    if (_val >= 0)
    {
      if (_show_plus) //показывать + или нет
        sprintf(buff, "+%s", str_buf);
      else sprintf(buff, " %s", str_buf);
    } else sprintf(buff, "%s", str_buf);
    lcd.setCursor(_x, _y);
    if (blink500ms)
    {
      for (int i = 0; i < strlen(buff); i++)
      {
        lcd.print(" ");
      }
    }
    else {

      lcd.print(buff);
      if (_icon) lcd.write(_icon); // значок градуса
    }
    key = read_buttons();
    if (key == btnUP) {
      _val += _step;

    }
    if (key == btnDOWN) {
      _val -= _step;
    }
    _val = constrain(_val, _min, _max); // крайние значения
  } while ( key != btnSELECT && key != btnRIGHT && key != btnLEFT  && MenuTimeoutTimer > 0 );
  if (MenuTimeoutTimer == 0 || (key == btnLEFT  && _leftesc )) //отмена изменений
  {
    _val = tmp;
    print();
  }
  return key;
}

void  CfgParamF::print()
{
  char buff[17];
  char str_buf[6];
  dtostrf(_val, 2, 1, str_buf);
  if (_val >= 0)
  {
    if (_show_plus) //показывать + или нет
      sprintf(buff, "+%s", str_buf);
    else sprintf(buff, " %s", str_buf);
  } else sprintf(buff, "%s", str_buf);
  lcd.setCursor(_x, _y);
  lcd.print(buff);
  if (_icon) lcd.write(_icon); // значок градуса
}

void CfgParamF::setpos(byte x, byte y) {
  _x = x; _y = y;
}

///////////////////////////////////////////////////////////////////////////////
CfgParamEEPROM::CfgParamEEPROM() {}
CfgParamEEPROM::~CfgParamEEPROM() {}
CfgParamEEPROM::CfgParamEEPROM(const int addr, float min = 15.0, float max = 30.0, float step = 0.1)
  : CfgParamF(min, min, max, step),
    _addr(addr),
    _autosave(true)
{
  _val = EEPROM.get(_addr, _val);
}

float CfgParamEEPROM::val(const float  v)
{
  _val = v;
  save();
  /*  if (_val != v)  {
      _val=v;
      if(_autosave){
        EEPROM.put(_addr,v);
        lcd.setCursor(0, 1);
        lcd.print(F("    Saved      "));
        delay(500);
      }

    }
  */
  return _val;

}
float CfgParamEEPROM::val()
{
  return CfgParamF::val();
}
void CfgParamEEPROM::save()
{
  //     float t= EEPROM.get(_addr,t);
  if (unsync()) //если есть рассинхрон с ЕЕПРЩМОМ
  {
    EEPROM.put(_addr, _val);
    if (_autosave)
    {
      lcd.setCursor(0, 1);
      lcd.print(F("    Saved      "));
      delay(500);
    }
  }
}

void CfgParamEEPROM::sync()
{
  EEPROM.get(_addr, _val);
}

boolean CfgParamEEPROM::autosave(boolean t)
{
  return _autosave = t;
}

int CfgParamEEPROM::edit()
{
  int key = CfgParamF::edit(); // вызов функции из род класса

  if ((key == btnSELECT || key == btnRIGHT) and _autosave ) save();
  return key;
}

boolean CfgParamEEPROM::unsync()
{
  float t = EEPROM.get(_addr, t);
  return t != _val;
}
///////////////////////////////////////////////////////////////////////////////
CfgParamSet::CfgParamSet()
{
}
CfgParamSet::~CfgParamSet()
{
}



CfgParamSet::CfgParamSet(const char v, char* s[], char size):
  _val(v), _set(s), _size(size), _maxlen(0), _x(0), _y(1), _leftesc(true)
{

  for (char i = 0; i < _size; i++)
  {
    int t = strlen(_set[i]);
    if (t > _maxlen) _maxlen = t;
  }

}

char CfgParamSet::val()
{
  return _val;
}

char CfgParamSet::val(const char v)
{
  return _val;
}

int CfgParamSet::edit()
{
  char tmp = _val;
  int key = btnNONE;

  do {
    lcd.setCursor(_x, _y);
    if (blink500ms) {
      for (int i = 0; i < _maxlen; i++)
      {
        lcd.print(" ");
      }

    }
    else {

      lcd.print(_set[_val]);
    }
    key = read_buttons();
    if (key == btnUP) {
      if (++_val > (_size - 1)) _val = 0;

    }
    if (key == btnDOWN) {
      if (--_val < 0) _val = _size - 1;
    }

  } while ( key != btnSELECT && key != btnRIGHT && key != btnLEFT  && MenuTimeoutTimer > 0 );
  if (MenuTimeoutTimer == 0 || (key == btnLEFT && _leftesc)) {
    _val = tmp;
    print();

  }
  return key;
}

void CfgParamSet::print()
{
  lcd.setCursor(_x, _y);
  lcd.print(_set[_val]);
}

void CfgParamSet::setpos(byte x, byte y)
{
  _x = x; _y = y;
}

boolean CfgParamSet::leftesc(boolean t)
{
  return _leftesc = t;
}
///////////////////////////////////////////
CfgParamSetE::CfgParamSetE()
{
}
CfgParamSetE::~CfgParamSetE()
{
}

CfgParamSetE::CfgParamSetE(const int addr, char* s[], char size): CfgParamSet(0, s, size), _addr(addr), _autosave(true)
{
  _val = EEPROM.get(_addr, _val);
}

char CfgParamSetE::val(const char v)
{

  _val = v;
  save();
  /*   if (_autosave) //сохранениие в EEPROM
     {
        float t = EEPROM.get(_addr,t);
        EEPROM.update(_addr,_val);
        if (v != t) //если были реальные изменения пишем трафарет
        {
         lcd.setCursor(0, 1);
         lcd.print(F("    Saved      "));
         delay(500);
        }
     }
  */
  return _val;
}

char CfgParamSetE::val()
{
  ; return CfgParamSet::val();
}

void CfgParamSetE::save()
{
  //   char t = EEPROM.get(_addr,t); //старое значение
  if (unsync())
  {
    EEPROM.update(_addr, _val);
    if (_autosave) // пишем трафарет
    {
      lcd.setCursor(0, 1);
      lcd.print(F("    Saved      "));
      delay(500);
    }
  }
}

void CfgParamSetE::sync()
{
  EEPROM.get(_addr, _val);
}

boolean CfgParamSetE::autosave(boolean t)
{
  return _autosave = t;
}
boolean CfgParamSetE::autosave()
{
  return _autosave;
}
int CfgParamSetE::edit()
{
  int key = CfgParamSet::edit(); // вызов функции из род класса

  if ((key == btnSELECT || key == btnRIGHT) and _autosave ) save();
  return key;
}

boolean CfgParamSetE::unsync() //есть ли несохраненные изменения
{
  char t = EEPROM.get(_addr, t); //старое значение
  return t != _val;
}
//Class Time//////////////////////////////////////////////////////////

Time::Time(): _h(0), _m(0), _leftesc(true) {}
Time::~Time() {}

Time::Time(byte h, byte m): _h(h), _m(m), _leftesc(true)
{
}
void Time::set(byte h, byte m)
{
  _h = h; _m = m;
}
///////////////////////////////////////////////////////////////////////
void Time::print()
{
  char buff[6];
  lcd.setCursor(_x, _y);
  sprintf_P(buff, (const char *)F("%02d:%02d"), _h, _m);
  lcd.print(buff);
}

////////////////////////////////////////////////////////////////////////////
int Time::edit()
{
  byte h_backup = _h, m_backup = _m;
  int key = btnNONE;
  byte mitem = 1; //1 - редактируем часы, 2 - минуты
  do {
    switch (mitem) {
      case 1:
        // ========= set hours
        do {

          key = read_buttons();
          if ( key == btnUP && ++_h > 23) _h = 0;
          if ( key == btnDOWN && --_h < 0) _h = 23;
          if ( key == btnLEFT) {
            --mitem ;/* _h=h_backup;*/
          }
          if ( key == btnRIGHT || key == btnSELECT ) ++mitem ;
          blinkCursor(0);

        } //do end
        while ( key != btnSELECT && key != btnLEFT && key != btnRIGHT && MenuTimeoutTimer > 0 );
        break;


      case 2:
        // ========= set minutes
        do {

          key = read_buttons();
          if ( key == btnUP && ++_m > 59) _m = 0;
          if ( key == btnDOWN && --_m < 0) _m = 59;
          if ( key == btnLEFT) {
            --mitem ; /*_m=m_backup;*/
          }
          if ( key == btnRIGHT || key == btnSELECT ) ++mitem ;
          blinkCursor(1);
        } //do end
        while ((key != btnSELECT) && (key != btnLEFT) && (key != btnRIGHT) && (MenuTimeoutTimer > 0));
        break;
    }//switch end
  }//do end
  while ( mitem > 0 && mitem < 3 && MenuTimeoutTimer > 0);
  if (MenuTimeoutTimer == 0 || (key == btnLEFT && _leftesc)) // если вышли по тамауту или по клавише ESC отменяем изм
  {
    _h = h_backup;
    _m = m_backup;
    print();
  }
  return key;
}//func end

/////////////////////////////////////////////////////////////////////////////////
void Time::blinkCursor(byte pos )
{
  char buff[3];
  lcd.setCursor(_x, _y);
  if ( !pos && blink500ms)
  {
    lcd.print(F("  "));
  }
  else {
    sprintf_P(buff, (const char *)F("%02d"), _h);
    lcd.print(buff);
  }
  lcd.print(F(":"));

  if (pos && blink500ms)
  {
    lcd.print(F("  "));
  }
  else {
    sprintf_P(buff, (const char *)F("%02d"), _m);
    lcd.print(buff);
  }

}
void Time::setpos(byte x, byte y)
{

  _x = x; _y = y;
}

boolean Time::leftesc(boolean t)
{
  return _leftesc = t;
}

char Time::h()
{
  return _h;
}

char Time::m()
{
  return _m;
}
/////////////////////////////////////////////////////////////
////TimeE
TimeE::TimeE(int addr): Time(),
  _addr(addr),
  _autosave(true) {
  EEPROM.get(_addr, _h);
  if (_h > 23 || _h < 0) _h = 0;
  EEPROM.get(_addr + 1, _m);
  if (_m > 59 || _m < 0) _m = 0;
}
TimeE::TimeE():
  _addr(1024),
  _autosave(false) {}


TimeE::~TimeE() {}

int TimeE::edit() {
  int key = Time::edit(); // вызов функции из род класса
  /*   if(key==btnLEFT) //отменяем изменения
     {
        sync();
        print();
     }
  */
  if ((key == btnSELECT || key == btnRIGHT) and _autosave ) save();
  return key;
}

void TimeE::sync()
{
  EEPROM.get(_addr, _h);
  EEPROM.get(_addr + 1, _m);
}
void TimeE::save()
{
  EEPROM.update(_addr, _h);
  EEPROM.update(_addr + 1, _m);

}

boolean TimeE::autosave()
{
  return _autosave;
}

boolean TimeE::autosave(boolean t)
{
  return _autosave = t;
}

void TimeE::set( byte h, byte m)
{
  _h = h; _m = m;
  if (_autosave)
  {
    EEPROM.put(_addr, _h);
    EEPROM.put(_addr, _m);
  }
}

boolean TimeE::unsync()
{
  byte h = EEPROM.get(_addr, h);
  byte m = EEPROM.get(_addr + 1, m);
  return _h != h || _m != m;
}



//Class Date//////////////////////////////////////////////////////////

Date::Date(): _d(1), _m(1), _yr(10), _leftesc(true) {}
Date::~Date() {}

Date::Date(byte d, byte m, byte y): _d(d), _m(m), _yr(y), _leftesc(true)
{
}
void Date::set(byte d, byte m, byte y)
{
  _d = d; _m = m, _yr = y;
}
///////////////////////////////////////////////////////////////////////
void Date::print()
{
  char buff[9];
  lcd.setCursor(_x, _y);
  sprintf_P(buff, (const char *)F("%02d-%02d-%02d"), _d, _m, _yr);
  lcd.print(buff);
}

////////////////////////////////////////////////////////////////////////////
int Date::edit()
{
  byte d_backup = _d, m_backup = _m, y_backup = _yr;
  int key = btnNONE;
  byte mitem = 1; //1 - редактируем день, 2 - месяц, 3-год
  do {
    switch (mitem) {
      case 1:
        // ========= set day
        do {

          key = read_buttons();
          if ( key == btnUP && ++_d > 31) _d = 1;
          if ( key == btnDOWN && --_d < 1) _d = 31;
          if ( key == btnLEFT) {
            --mitem ;/* _h=h_backup;*/
          }
          if ( key == btnRIGHT || key == btnSELECT ) ++mitem ;
          blinkCursor(0);

        } //do end
        while ( key != btnSELECT && key != btnLEFT && key != btnRIGHT && MenuTimeoutTimer > 0 );
        break;


      case 2:
        // ========= set month
        do {

          key = read_buttons();
          if ( key == btnUP && ++_m > 12) _m = 1;
          if ( key == btnDOWN && --_m < 1) _m = 12;
          if ( key == btnLEFT) {
            --mitem ; /*_m=m_backup;*/
          }
          if ( key == btnRIGHT || key == btnSELECT ) ++mitem ;
          blinkCursor(1);
        } //do end
        while ((key != btnSELECT) && (key != btnLEFT) && (key != btnRIGHT) && (MenuTimeoutTimer > 0));
        break;

      case 3:
        // ========= set year
        do {

          key = read_buttons();
          if ( key == btnUP && ++_yr > 99) _yr = 0;
          if ( key == btnDOWN && --_yr < 0) _yr = 99;
          if ( key == btnLEFT) {
            --mitem ; /*_m=m_backup;*/
          }
          if ( key == btnRIGHT || key == btnSELECT ) ++mitem ;
          blinkCursor(2);
        } //do end
        while ((key != btnSELECT) && (key != btnLEFT) && (key != btnRIGHT) && (MenuTimeoutTimer > 0));
        break;
    }//switch end
  }//do end
  while ( mitem > 0 && mitem < 4 && MenuTimeoutTimer > 0);
  if (MenuTimeoutTimer == 0 || (key == btnLEFT && _leftesc)) // если вышли по тамауту или по клавише ESC отменяем изм
  {
    _d = d_backup;
    _m = m_backup;
    _yr = y_backup;
    print();
  }
  return key;
}//func end

/////////////////////////////////////////////////////////////////////////////////
void Date::blinkCursor(byte pos )
{
  char buff[3];
  lcd.setCursor(_x, _y);

  if ( pos == 0 && blink500ms)
  {
    lcd.print(F("  "));
  }
  else {
    sprintf_P(buff, (const char *)F("%02d"), _d);
    lcd.print(buff);
  }
  lcd.print(F("-"));

  if (pos == 1 && blink500ms)
  {
    lcd.print(F("  "));
  }
  else {
    sprintf_P(buff, (const char *)F("%02d"), _m);
    lcd.print(buff);
  }
  lcd.print(F("-"));

  if (pos == 2 && blink500ms)
  {
    lcd.print(F("  "));
  }
  else {
    sprintf_P(buff, (const char *)F("%02d"), _yr);
    lcd.print(buff);
  }


}
void Date::setpos(byte x, byte y)
{

  _x = x; _y = y;
}

boolean Date::leftesc(boolean t)
{
  return _leftesc = t;
}

char Date::d()
{
  return _d;
}

char Date::m()
{
  return _m;
}
char Date::y()
{
  return _yr;
}

/////////////////////////////////////////////////////////////

