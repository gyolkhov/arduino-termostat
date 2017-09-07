#ifndef _MISC_H_
#define _MISC_H_

#define PINGCMD 85  
#define PINGANSWERCMD 86
#define RELAYCMD 88
#define RELAYON 99999999
#define RELAYOFF 88888888
#define CHAR_SUN 1
#define CHAR_DEGREE 2
#define CHAR_ANTENNA 3
#define CHAR_LEVEL 4
#define CHAR_LEVEL0 5
#define CHAR_LEVEL01 6
struct Radiodata // структура для отправки команд по радио
{ 
   uint16_t cmd; //команда
   unsigned long data ;//данные long выбран для возможности отправки  millis() в команде пинг
 };

enum states {ON, OFF};

#endif

