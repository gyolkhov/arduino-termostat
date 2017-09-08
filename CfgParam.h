#ifndef _CFGPARAM_H_
#define _CFGPARAM_H_
#include <Arduino.h>


class CfgParamF //Параметр с типом Float
{
  protected:

    float _val;
    float _min;
    float _max;
    float _step;
    boolean _show_plus;
    char _icon; // значок величины, например значок градуса. Если ноль - не печатать ничего
    byte _x;
    byte _y;
    boolean _leftesc;
  public:

    CfgParamF(const float v, float min = 15.0, float max = 30.0, float step = 0.1);
    CfgParamF();
    virtual ~CfgParamF();
    virtual float val( const float t);
    virtual float val();
    float minval();
    float minval(const float t);
    float maxval();
    float maxval(const float t);
    float step();
    float step(const float);
    virtual int edit();
    boolean show_plus(boolean t);
    boolean leftesc(boolean t);
    char icon(char t);
    void print();
    void setpos(byte x, byte y);


};

class CfgParamEEPROM: public CfgParamF  //с сохранением в EEPROM
{
  private:
    int _addr;
    boolean _autosave;
    CfgParamEEPROM();

  public:

    CfgParamEEPROM(const int addr, float min, float max, float step);
    virtual ~CfgParamEEPROM();
    virtual float val( const float t);
    virtual float val();
    void save();
    void sync();
    boolean autosave(boolean t);
    int edit();
    boolean unsync();
};

/////////////////////////////////////////////////////////////////
class CfgParamSet //класс с типом целое (char), которое является индексом массива символов  set
{
  protected:
    char _val;
    char** _set;
    char _size;
    int _maxlen;
    byte _x;
    byte _y;
    boolean _leftesc;
    CfgParamSet();
  public:

    CfgParamSet(const char v, char* s[], char size);
    virtual ~CfgParamSet();
    virtual char val();
    virtual char val( const char t);
    virtual int  edit();
    void setpos(byte x, byte y);
    virtual void  print();
    boolean leftesc(boolean t);

};

class CfgParamSetE: public CfgParamSet  //класс с типом целое (char), которое является индексом массива символов  set
{
  protected:
    int _addr;
    boolean _autosave;
    CfgParamSetE();
  public:

    CfgParamSetE(const int addr, char* s[], char size);
    virtual ~CfgParamSetE();
    virtual char val( const char t);
    virtual char val();
    boolean autosave();
    boolean autosave(boolean t);
    void save();
    void sync();
    int edit();
    boolean unsync();

};

/////////////////////////////////////////////////

class Time
{
  protected:
    char _h;
    char _m;
    byte _x;
    byte _y;
    boolean _leftesc;

    void blinkCursor(byte h);
  public:
    Time(byte h, byte m);
    Time();
    virtual ~Time();
    void print();
    virtual void set(byte h, byte m);
    virtual int edit();
    void setpos(byte x, byte y);
    boolean leftesc(boolean t);
    char h();
    char m();
};
///////////////////////////////////////////////////////////////////////////
class TimeE: public Time
{
  private:
    int _addr; //адресс в EEPROM
    boolean _autosave;

  public:
    TimeE();
    TimeE(int addr);
    void save();
    void sync();
    boolean autosave();
    boolean autosave(boolean t);
    int edit();
    void set(byte h, byte m);
    boolean unsync();
    virtual ~TimeE();


};


class Date
{
  protected:
    char _d;
    char _m;
    char _yr;
    byte _x;
    byte _y;
    boolean _leftesc;

    void blinkCursor(byte p);
  public:
    Date(byte d, byte m, byte y);
    Date();
    virtual ~Date();
    void print();
    virtual void set(byte d, byte m, byte y);
    virtual int edit();
    void setpos(byte x, byte y);
    boolean leftesc(boolean t);
    char d();
    char m();
    char y();
};
#endif
