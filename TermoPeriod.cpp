#include "TermoPeriod.h" 
#include "Buttons.h"
#include <LiquidCrystal.h> // LCD 16*2
extern LiquidCrystal lcd; //Глоб перемен определенные в другом файле
extern volatile boolean blink500ms;
extern volatile byte MenuTimeoutTimer; //Глобальная переменная определенная в другом файле
 //////////////////////////////////////////////////////////////////
 /////////Termoperiod/////////////////////////////////////
 TermoPeriod::TermoPeriod():_temp(1024,20,30,0.1),_active(0,_menu,2)
 {
 }
 TermoPeriod::~TermoPeriod()
 {
 }
TermoPeriod::TermoPeriod(const byte  i, const int addr):
 _id(i),_addrt1(addr),_addrt2(addr+2),
 _addractive(addr+4),_addrtemp(addr+5),
 _t1(_addrt1),
  _t2(_addrt2),
  _temp(_addrtemp,15,30,0.1),
  _active(_addractive,_menu,2)
 
 {

   _t1.autosave(false);
   _t1.leftesc(false);
   _t2.autosave(false);
   _t2.leftesc(false);
   _temp.autosave(false);
   _temp.leftesc(false);
  _active.autosave(false);
   _active.leftesc(false);
 }

  void TermoPeriod::set( const byte h_1, const byte  m_1,const byte h_2, const byte m_2, const float t, const char   a)
  {  
    _t1.set(h_1,m_1);
    _t1.save();
    _t2.set(h_2,m_2);
    _t2.save();
    _temp.val(t);
    _temp.save();
    _active.val(a);  
    _active.save();
    
  }
int TermoPeriod::edit()
 {
  char buff[17];
//  sync(); //считывание полей с EEPROM
  lcd.clear(); 
  lcd.setCursor(0, 0);
  sprintf_P(buff, (const char *)F("P%d=["), _id);
  lcd.print(buff);
  _t1.setpos(4,0);
  _t1.print();
  lcd.print("-");
  _t2.setpos(10,0);
  _t2.print();
  lcd.print("]");
  lcd.setCursor(0, 1);
  lcd.print("t=");
  _temp.setpos(2,1);
  _temp.print();
  _active.setpos(9,1);
  _active.print();
  
  byte mitem = 1;
  int key = btnNONE;
  do {
    switch (mitem) {
      case 1: // Установка времени начала периода
        key=_t1.edit();
        if (key == btnSELECT || key == btnRIGHT) {
          mitem++;
        }
        if (key == btnLEFT) mitem--;
        _t1.print();
        break;

      case 2:  // Установка времени конца периода
         key=_t2.edit();
        if (key == btnSELECT || key == btnRIGHT) {
          mitem++;
        }
        if (key == btnLEFT) mitem--;
        _t2.print();
        break;

      case 3: //установка температуры периода
        key=_temp.edit();
        if (key == btnSELECT || key == btnRIGHT) {
          mitem++;
        }
        if (key == btnLEFT) mitem--;
        _temp.print();
       
        break ;

      case 4: // ВКЛ-ВЫКЛ периода


        key=_active.edit();
        if (key == btnSELECT || key == btnRIGHT) {
          mitem++;
        }
  //      if (key == btnRIGHT && mitem > 4) mitem = 1; //ходим по кругу полей
        if (key == btnLEFT) mitem--;
        _active.print();

       
        break ;


 
    } //switch end

  } //do end
  while ( mitem > 0 && mitem < 5 && MenuTimeoutTimer > 0);
  if(MenuTimeoutTimer==0)
  {
     sync(); //синхронизируем с EEPROMом то есть отменяем все изменения в полях объекта
  } else 
  if (unsync()) //есть реальные изменения
  {
    if (yes_no_dialog())//Запрос на сохранение/отмену изменений
    {
      save(); //Сохраняем изменения в EEPROM

      lcd.setCursor(0, 1);
      lcd.print(F("     Saved     "));
      delay(500);
    }else
    {
      sync(); //синхронизируем с EEPROMом то есть отменяем все изменения в полях объекта
    }
  }
 
  lcd.setCursor(0, 0);
  lcd.print("<SETUP>         ");
  return key;
 }

 /////////////////////////////////////////////////////////////////////////////////////////
void TermoPeriod::save()
{
  _t1.save();
  _t2.save();
  _temp.save();
  _active.save();
}

////////////////////////////////
void TermoPeriod::sync()
{
  _t1.sync();
  _t2.sync();
  _temp.sync();
  _active.sync();
}
//////////////////////////////////////
boolean TermoPeriod::unsync()
{
  return _t1.unsync() || _t2.unsync() ||_temp.unsync() || _active.unsync();
}
///////////////////////////////////////////
boolean TermoPeriod::yes_no_dialog()
{
      int key=btnNONE;
      boolean  retval=true;
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print(F("     Save?     "));
      do {

        key = read_buttons();

        
        if (key == btnUP || key == btnDOWN )  retval=!retval;
       

        // индикация пункта меню (номер пункта - в menuitem)
        lcd.setCursor(0, 1); //инфо на LCD
        if (retval)
        {
            lcd.print(F("      YES      "));
        } else
        {
            lcd.print(F("      NO      "));     
        }
      }
      while (key != btnSELECT  && MenuTimeoutTimer > 0);

      if (MenuTimeoutTimer == 0) {
        retval = false;
      }
      return retval;
}

bool TermoPeriod::active()
{
  return (true?_active.val()==0:false);
}
char TermoPeriod::h1()
{
  return _t1.h();
}

char TermoPeriod::h2()
{
  return _t2.h();
}

char TermoPeriod::m1()
{
  return _t1.m();
}

char TermoPeriod::m2()
{
  return _t2.m();
}

float TermoPeriod::temp()
{
  return _temp.val();
}
