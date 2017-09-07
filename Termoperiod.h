#ifndef _TERMOPERIOD_H_
#define _TERMOPERIOD_H_
#include <EEPROM.h>
#include <Arduino.h>
#include "CfgParam.h"

//////////////////////////////////////
class TermoPeriod
{
  private: 
    char* _menu[2]={"On ","Off"};  
    int _addrt1;
    int _addrt2;
    int _addrtemp;
    int _addractive;
    TimeE _t1;
    TimeE _t2;
    CfgParamEEPROM _temp;
    CfgParamSetE _active;
    byte  _id;
    TermoPeriod();  
  public:
    
    TermoPeriod(const byte i, const int eeaddr);
    virtual ~TermoPeriod();
    byte id();
    float temp();
    bool  active();
    float temp( const float t);
 //   char active(const char  a); 
    void set( const byte h_1, const byte  m_1,const byte h_2, const byte m_2, const float t, const char  a);
    int edit();
    void save();
    void sync();
    boolean yes_no_dialog();
    boolean unsync();
    char h1();
    char h2();
    char m1();
    char m2();
    
    
  
};
/////////////////////////////////
#endif

