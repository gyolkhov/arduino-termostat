#ifndef _TERMOSTAT_H_
#define _TERMOSTAT_H_
#include <Arduino.h>
#include "Misc.h"
#include "TermoPeriod.h" // температурный период  класс
#include "CfgParam.h" //  классы для  настоечных параметров , например, температура теростата, гистерезис
//#include "Buttons.h" //определение кнопок и функции считывания значения кнопок
#include "EEPROMAddresses.h" //определение адресов EEPROM


class Termostat
{
  public:  
 
    states relay; // состоние удаленного реле
    float temper; // текущая температура 
    int  period; // здесь хранится индекс текущего периода
    bool relay_ack; //была ли подтверждена приемником последняя команда к реле Далее предполагается использовать для значка сосотяния радиосети 
    Termostat();   
    virtual ~Termostat();    
    bool relay_ctl(states) ; //управление  удаленным реле , вернее, вкл/выкл по радио
     //Гистререзис               min max step
    CfgParamEEPROM hyster; //
    //Коррекция датчика температуры
    CfgParamEEPROM tempCorr;
    //Алармовая температура
    CfgParamEEPROM tempAlarm; 
    //массив термопериодов
    TermoPeriod termoPeriod[4];

    CfgParamEEPROM tempMan; //Температура при ручном режиме
      
    CfgParamSetE beep; 

    CfgParamSet mrelay;  // для управления вруную реле
 
    CfgParamSetE mode; //  
    //Нахождение текущего термопериода и возвращение его индекса в массиве. В качестве аргументов -  час и минута
    int findTermoPeriod(int,int);
};

#endif
