#include "Termostat.h"
#include <nRF24L01.h>
#include <RF24.h>

extern RF24 radio;

static char* strmenuOnOff[] = {"On ", "OFF"}; //Меню выбора для настроек звука
static char* strmenuMode[] = {"AUTO", "MAN "}; //Меню выбора для настроек режима термостата

Termostat::Termostat():
        relay(OFF),
        relay_ack(false),
        hyster(_HYSTER_ADDR, 0, 2.0, 0.1),
        tempCorr(_TEMPCORR_ADDR, -3.0, 3.0, 0.1),
        tempAlarm(_TEMPALARM_ADDR, 15.0, 30.0, 1.0),
        tempMan(_TEMPMAN_ADDR,0.0,26.0,0.1),
        termoPeriod(
          {
           TermoPeriod(1, _TERMO_PR1_ADDR),
           TermoPeriod(2, _TERMO_PR2_ADDR),
           TermoPeriod(3, _TERMO_PR3_ADDR),
           TermoPeriod(4, _TERMO_PR4_ADDR)
          }
        ),
        beep(_BEEP_ADDR,strmenuOnOff, 2),
        mrelay(1,strmenuOnOff, 2),
        mode(_MODE_ADDR, strmenuMode, 2)
{  
}

Termostat::~Termostat()
{  
}

bool Termostat::relay_ctl(states st)
{ 
   Radiodata rd;
   radio.setPayloadSize(sizeof(Radiodata));
   rd.cmd=RELAYCMD; 
   if(st == ON){
    rd.data  = RELAYON;
   }else{
    rd.data  = RELAYOFF;
   }
  relay=st; //статус 
  for (byte i = 0; i < 10; i++)
  {  
     relay_ack=false;
     radio.stopListening();
    if (radio.write( &rd, sizeof(rd)))
    {
      relay_ack=true;
      Serial.print("Sent RELAY =: ");
      Serial.println(rd.data);
      return true;
    }
    Serial.println("sent error ");
    delay(1);

  }
  return false; 
}

//Нахождение текущего термопериода и возвращение его индекса в массиве. В качестве аргументов -  час и минута
 int Termostat::findTermoPeriod(int h,int m)
 {
   if(mode.val()==1) {return period=-1;} //если руной режим
   for(int i=0;i<4;i++)
   {
       if(termoPeriod[i].active())
       {
         char h1=termoPeriod[i].h1();
         char h2=termoPeriod[i].h2();
         char m1=termoPeriod[i].m1();
         char m2=termoPeriod[i].m2();
         int tStart=h1*60+m1;
         int tEnd=h2*60+m2; 
         int t=h*60+m;    // текущая минута  
         if(tEnd<tStart)
         {
             tEnd=tEnd+24*60;
             if(t<tStart)
             {
                 t+=24*60;
             }
             
         }
          
         if(t>=tStart && t<=tEnd)
         {
              ; //сохраняем 
              return period=i;
         }
       }
   }
   return period=-1;
 }

