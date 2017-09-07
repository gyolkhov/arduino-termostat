


 /*
  Суточный термостат версия 1.0
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h> // https://github.com/maniacbug/RF24
#include <iarduino_RTC.h>
#include <OneWire.h> // 1wire для DS18B20
#include <DallasTemperature.h> // DS18B20
#include <EEPROM.h> // EE
#include <LiquidCrystal.h> // LCD 16*2
#include <TimerThree.h> // прерывания по таймеру1
#include "TermoPeriod.h" // температурный период  класс
#include "CfgParam.h" //  классы для  настоечных параметров , например, температура теростата, гистерезис
#include "Buttons.h" //определение кнопок и функции считывания значения кнопок
#include "EEPROMAddresses.h" //определение адресов EEPROM
#include "Misc.h" //различные мои вспомогательные структуры 
#include "Termostat.h"
#define ONE_WIRE_BUS 22


const  uint8_t pipes[][6] = {"1Node", "2Node"};

//RTC  RTCtime; //RTC time;
iarduino_RTC RTCtime(RTC_DS3231);
OneWire OneWire(ONE_WIRE_BUS);
DallasTemperature Tsensors(&OneWire);
// LCD connection RS, E, D4, D5, D6, D7
// R/W - to ground
//Настройка дисплея
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


// псевдограф символы
byte customChar1[8] = {
  0b00100,
  0b10101,
  0b01110,
  0b11111,
  0b01110,
  0b10101,
  0b00100,
  0b00000
}; // звезда

byte customChar2[8] = {
  0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00
}; // значок градуса

byte customChar3[8] = {
  0b10001,
  0b10101,
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b01110,
  0b00000
}; //антенна

byte customChar4[8] = {
  0b00000,
  0b00000,
  0b00010,
  0b00010,
  0b00110,
  0b01110,
  0b01110,
  0b00000
};//уровень сигнала

byte customChar5[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10101,
  0b00000
}; // Нулевой уроветь сигнала

byte customChar6[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
RF24 radio(48, 53); // CE, CSN

Termostat TS; // Главная переменная термостата

unsigned int TstatTimer = 20; //таймер паузы между включениями/выключениями, начальная установка 20 сек для устаканивания системы после сброса
volatile boolean blink500ms = false; // мигающий бит, инвертируется каждые 500мс
volatile boolean plus1sec = false; // ежесекундно взводится
volatile byte MenuTimeoutTimer; //Выход из меню бессохранения по неактивности
volatile boolean TempNeedUpdate = true; //флаг необходимости обновления показания температуры
volatile boolean EveryMinute = true;
/////////////////////////////////////////////////////////////////////////////////////////////
// ===== SETUP ========================================================================
//////////////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(300);
  //#ifdef serialenabled
  Serial.begin(9600);
  //#endif
  RTCtime.begin();
  RTCtime.period(1);
  Tsensors.begin();
  Tsensors.setResolution(12);
  //TS.temper=getTemperature();

  radiosetup();

  // pinMode(Relay, OUTPUT);
  // digitalWrite(Relay, HIGH);
  lcd.begin(16, 2);
  lcd.createChar(CHAR_SUN, customChar1); //солнце
  lcd.createChar(CHAR_DEGREE, customChar2);//градус
  lcd.createChar(CHAR_ANTENNA, customChar3);//аннтена
  lcd.createChar(CHAR_LEVEL, customChar4);//уровень сигнала
  lcd.createChar(CHAR_LEVEL0, customChar5);//нулевой уровень сигнала
  lcd.createChar(CHAR_LEVEL01, customChar6);// второй нулевой уровень сигнала для эффекта бег огни
  
  Timer3.initialize(500000); // Timer0 interrupt - set a timer of length 500000 microseconds
  Timer3.attachInterrupt( timerIsr ); // attach the service routine here
  if (EEPROM.read(_MAGIC_WORD_ADDR) != 112) { // если первая запись EEPROM - записать начальные значения в EE
    EEPROM.write(_MAGIC_WORD_ADDR, 112);
    TS.beep.val(1);
    TS.tempCorr.val(0);
    TS.hyster.val(0.5);
    TS.tempAlarm.val(26);
    //            h1 m1 h2 m2  t  on/off
    TS.termoPeriod[0].set(0, 0, 5, 59, 21, 0);
    TS.termoPeriod[1].set(6, 0, 7, 59, 23, 0);
    TS.termoPeriod[2].set(8, 0, 16, 59, 22, 0);
    TS.termoPeriod[3].set(17, 0, 23, 59, 23, 0);
    TS.mode.val(0);
    TS.tempMan.val(22);
  }
  TS.tempCorr.show_plus(true); //показывать + или -
  // TempCorr.icon(0);
}
///////////////////////////////////////////////////////////////////////////////////////
///===== MAIN CYCLE ================================================================///
///////////////////////////////////////////////////////////////////////////////////////
void loop() 
{
  char menuitem = 1;
  boolean menuexit = false;
  int key = btnNONE;
  key = read_buttons(); // Считывание значения кнопки
  if (key == btnNONE) // Полезная работа тут
  {
    if (TempNeedUpdate) // необходимо обновит показания температуры
    {
      TS.temper = getTemperature();
      Serial.print("Temperature=");
      Serial.println(TS.temper);
    }
    printSplash(); // Заставка
    if (EveryMinute) //каждую минуту  проверяем температурные условия
    {
       EveryMinute=false;
       //Тут полезная работа термостата
       usefulWork();
    }
  } else

    // ================ по нажатию кнопки RIGHT или SELECT - меню настроек ====================
    if ( key == btnRIGHT || key == btnSELECT) {
      do { //цикл меню
        lcd.clear();
        lcd.setCursor(0, 0); //инфо на LCD
        lcd.print(F("< SETUP >"));
        do {
          key = read_buttons();
          if (key != btnNONE)
          {
            if (key == btnUP)  menuitem--;
            else if (key == btnDOWN)   menuitem++;
            if ( menuitem > 12 ) {
              menuitem = 0;  // границы пунктов меню
            }
            if ( menuitem < 0 ) {
              menuitem = 12;
            }
          }

          // индикация пункта меню (номер пункта - в menuitem)
          lcd.setCursor(0, 1); //инфо на LCD
          switch (menuitem)
          {
            case 0:
              lcd.print(F("0.EXIT FROM MENU"));
              break;
            case 1:
              lcd.print(F("1.MODE SET      "));
              break;
            case 2:
              lcd.print(F("2.TEMPER MAN SET"));
              break;
            case 3:
              
              lcd.print(F("3.PERIODS       "));
              break;
            case 4:
              lcd.print(F("4.CLOCK SET TIME"));
              break;

            case 5:
              lcd.print(F("5.CLOCK SET DATE"));
              break;
            case 6:
              lcd.print(F("6.HYSTERESIS SET"));
              break;
            case 7:
              lcd.print(F("7.T-CORRECT SET "));
              break;
            case 8:
              lcd.print(F("8.SOUND SET     "));
              break;
            case 9:
              lcd.print(F("9.T-ALARM SET   "));
              break;
            case 10:
              lcd.print(F("10.BRIGHTNESS  "));
              break;
            case 11:
              lcd.print(F("11.PING         "));
              break;
            case 12:
              lcd.print(F("12.RELAY        "));
              break;
            
          }
        }
        while (key != btnSELECT && key != btnLEFT && MenuTimeoutTimer > 0);

        if (MenuTimeoutTimer == 0 || key == btnLEFT) {
          menuitem = 0;
        }
        // если нажата кнопка SELECT или таймаут - обработка пункта меню (номер пункта - в menuitem)

        switch (menuitem)
        {
          // ====== пункт 0 - выход
          case 0:
            menuexit = true;
            break; // case 0 out

          // ====== пункт 1 - установка режима термостата
          case 1:

            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print("Termostat Mode:      ");
            TS.mode.setpos(7, 1);
            TS.mode.edit();

            break; // case 1 out

          // ====== пункт 2 - установка Температуры при ручном режиме
          case 2:
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print(F("SET TEMPERATURE "));
            TS.tempMan.setpos(5, 1);
            TS.tempMan.edit();
            break; // case 3 out

          // ====== пункт 3 - подменю установок временных периодов
          case 3:
            subMenuPeriods();
            break; // case 3 out
          // ====== пункт 4 - установка RTC
          case 4:
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print(F("SET CLOCK TIME"));
            /*
                      //      RTC.readClock();
                      //     Hours=RTC.getHours();
                      //    Minutes=RTC.getMinutes();
                      SetYesNo = false;
                      PrintYesNo = true;
                      SetTime(0, 1); // в позиции 0,1 - запрос ввода времени
                      if (MenuTimeoutTimer != 0) {
                        if (SetYesNo)
                        {

                          //         RTC.setHours(Hours);
                          //         RTC.setMinutes(Minutes);
                          //         RTC.setSeconds(0);
                          //        RTC.setClock();
                        }
                      }
            */
             setClockTime();
            break; // case 4 out
            
           case 5: // Установка даты
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print(F("SET CLOCK DATE"));
            setClockDate();
            break;
           
          // ====== пункт 6 - установка гистерезиса
          case 6:
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print(F("SETUP HYSTERESIS"));
            TS.hyster.setpos(5, 1) ; //позиция вывода на экран
            TS.hyster.edit();
            break; // case 6 out
          // ====== пункт 6 - установка коррекции температуры
          case 7:
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print(F("SETUP T-CORRECT "));
            TS.tempCorr.setpos(5, 1); //позиция вывода на экран
            TS.tempCorr.edit();
            break; // case 7 out
          // ====== пункт 7 - вкл/выкл звука
          case 8:
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print("   Sound set   ");
            lcd.setCursor(0, 1);
            lcd.print("Beep = ");
            TS.beep.setpos(7, 1);
            TS.beep.edit();
            break; // case 8 out
          // ====== пункт 8 - установка alarm температуры
          case 9:
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print(F("ALARM-TEMP SET  "));
            TS.tempAlarm.setpos(5, 1);
            TS.tempAlarm.edit();
            break; // case 9 out
          case 10:
            Brightness();
            break; // case 10 out

          case 11:
            radioping();
            lcd.setCursor(0,1);
            lcd.print(F("10.PING         "));
            break; // case 11 out
          case 12:
            //char rstat=TS.mrelay.val(); //запоминаем предыдущее состояние;
            lcd.clear();
            lcd.setCursor(0, 0); //инфо на LCD
            lcd.print("   CTRL RELAY  ");
            lcd.setCursor(0, 1);
            lcd.print("Relay = ");
            TS.mrelay.setpos(8, 1);
            TS.mrelay.edit();
            TS.relay_ctl(TS.mrelay.val()==0?ON:OFF);
            /*
            if(rstat != TS.mrelay.val()){
              relay_ctrl(TS.mrelay.val()==0?true:false);
            }
            */
            break;
 
        }
        MenuTimeoutTimer = 10;
      } while ( menuexit == false && MenuTimeoutTimer > 0);

      lcd.clear();
    }
}

// ===== SUBROUTINES ==================================================================



// ============================ Timer0 interrupt =============================
// run every 500ms
void timerIsr()
{
  static unsigned long sec = 0; //счетчик  секунд с момента включения
  blink500ms = !blink500ms; // инверсия мерцающего бита
  if (blink500ms) {
    sec++;
    if (sec % 10 == 0) TempNeedUpdate = true; // каждые 10 сек взводится флаг необходимости обновления температуры
    if (sec % 60 == 0) EveryMinute = true; // каждые 60 сек взводится флаг 
    plus1sec = true; // ежесекундно взводится
    if (TstatTimer != 0) {
      TstatTimer --; // ежесекундный декремент этого таймера
    }
    if (MenuTimeoutTimer != 0) {
      MenuTimeoutTimer --; // ежесекундный декремент этого таймера
    }
  }
}

//+++++ мигающий текст++++++++++++++++
void lcdPrintBlink(int x, int y, const __FlashStringHelper* t1, const __FlashStringHelper* t2) {
  lcd.setCursor(x, y);
  if (blink500ms) {
    lcd.print(t1);
  } else {

    lcd.print(t2);
  }
}



//+++++++++++++++++++++++++++++++++++++++++++
void Brightness()
{
  static int bri = 200;
  int tmp = bri;
  int key = btnNONE;
  lcd.clear();
  lcd.setCursor(0, 0); //инфо на LCD
  lcd.print(F("BRIGHTNESS    "));
  do {
    lcd.setCursor(0, 1);
    if (tmp < 100) {
      lcd.print(F(" "));
    }
    lcd.print(tmp);


    key = read_buttons();
    if (key == btnUP) {
      tmp += 10;

    }
    if (key == btnDOWN) {
      tmp -= 10;
    }
    tmp = constrain(tmp, 10, 255); // крайние значения
    analogWrite(10, tmp);
  } while ( key != btnSELECT  && key != btnLEFT  && MenuTimeoutTimer > 0 );
  if (key == btnSELECT && bri != tmp) {
    bri = tmp;
    lcd.setCursor(0, 1);
    lcd.print(F("    Saved      "));
    delay(500);
  }
  analogWrite(10, bri);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void subMenuPeriods() {
  char menuitem = 1;
  int key = btnNONE;
  do {
    do {

      key = read_buttons();

      if (key != btnNONE) {
        if (key == btnUP)  menuitem--;
        else if (key == btnDOWN)   menuitem++;
        if ( menuitem > 4 ) {
          menuitem = 1;  // границы пунктов меню
        }
        if ( menuitem < 1) {
          menuitem = 4;
        }
      }

      // индикация пункта меню (номер пункта - в menuitem)
      lcd.setCursor(0, 1); //инфо на LCD
      switch (menuitem)
      {
        case 1:
          lcd.print(F("3.1 PERIOD 1   "));

          break;
        case 2:
          lcd.print(F("3.2 PERIOD 2   "));
          break;
        case 3:
          lcd.print(F("3.3 PERIOD 3   "));
          break;
        case 4:
          lcd.print(F("3.4 PERIOD 4    "));
          break;

      }
    }
    while (key != btnSELECT && key != btnLEFT && MenuTimeoutTimer > 0);

    if (MenuTimeoutTimer == 0 || key == btnLEFT) {
      return;
    }
    switch (menuitem)
    {
      // ====== пункт 0 - выход
      case 1:
        TS.termoPeriod[0].edit();

        break; // case 1 out
      case 2:
        TS.termoPeriod[1].edit();
        break; // case 2 out
      case 3:
        TS.termoPeriod[2].edit();
        break; // case 3 out
      case 4:

        TS.termoPeriod[3].edit();
        break; // case 4 out
    }

  } while (MenuTimeoutTimer > 0);
}
/////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void printSplash()
{
  char buff[16];
  char strtemp[5];
  dtostrf(TS.temper, 2, 1, strtemp);
  // char* weekdays[]={"MON","Tue","Wed","Thu","Fri","Sat","SUN"};
  //    if(millis()%1000==0){ // если прошла 1 секунда
  if ( blink500ms ) // раз в секунду
  {
    //инфо на LCD
    RTCtime.gettime();
    lcd.setCursor(0, 0);

    sprintf_P(buff, (const char *)F("%02d-%02d-%02d"), RTCtime.day, RTCtime.month, RTCtime.year);
    lcd.print(buff);
    lcd.print(" ");
    if(TS.relay == ON) {
      lcd.write(CHAR_SUN); // значок солнца}
    }
    else{
      lcd.print(" ");
    }
    lcd.print(" ");
    if(TS.mode.val()==1) //в ручном режиме
    {
      lcd.print("M ");
    }else{
      if (TS.period == -1)
      {
        lcd.print("P-");
      }else{
        lcd.print("P");
        lcd.print(TS.period+1);
      }
    }
    lcd.setCursor(14, 0);
    lcd.write(CHAR_ANTENNA);
    if(TS.relay_ack){
       lcd.write(CHAR_LEVEL); 
    }
    else{
         
        lcd.write(CHAR_LEVEL0); 
    }
    
    sprintf_P(buff, (const char *)F("%02d:%02d:%02d t=%s"), RTCtime.Hours, RTCtime.minutes, RTCtime.seconds, strtemp);
    lcd.setCursor(0, 1);
    lcd.print(buff);
    lcd.write(CHAR_DEGREE);
    //    lcd.print(time.gettime("d-m-Y"));
    //   lcd.print(time.gettime("H:i:s"));
    //    Serial.println(time.gettime("d-m-Y, H:i:s, D")); // выводим время
    delay(1); // приостанавливаем на 1 мс, чтоб не выводить время несколько раз за 1мс
  }
}
/////////////////////////////////////
//Получить текущую температуру в помещении
float getTemperature()
{
  Tsensors.requestTemperatures();
  float t = Tsensors.getTempCByIndex(0);
  TempNeedUpdate = false;
  t = avrgTemp(t);
  Serial.print("t=");
  Serial.println(t);

  Serial.print("TempCorr.val() =");
  Serial.println(TS.tempCorr.val());
  return t + TS.tempCorr.val();
}
//////////////////////////////////////////////////////////////////
float avrgTemp(const float temp)//средняя температура за 6 периодов
{
  static float  temps[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  char strtemp[7];
  float sum = 0.0;
  //создаем очередь. Первый вошел - последний вышел(вытолкнут)
  //  Serial.println(temp);
  for (byte i = 5; i > 0; i--)
  {

    temps[i] = temps[i - 1];

  }

  temps[0] = temp;
  byte size = 0;
  for (byte i = 0; i < 6; i++)
  {
    Serial.println(temps[i]);
    if (temps[i] > 0.0)
    {
      sum = sum + temps[i];
      size++;
    }
  }

  Serial.print("sum = ");
  Serial.println(sum);
  if (size == 0) return 0.0;
  //округление
  dtostrf(sum / size, 2, 1, strtemp);
  return atof(strtemp);

}
///////////////////////////////////////////////////////////
void radiosetup()
{
  
  radio.begin();
  delay(2);
  radio.setChannel(9); // канал (0-127)

  // скорость, RF24_250KBPS, RF24_1MBPS или RF24_2MBPS
  // RF24_250KBPS на nRF24L01 (без +) неработает.
  // меньше скорость, выше чувствительность приемника.
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(true);
  radio.setRetries(5, 15);

  // мощьность передатчика, RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM,
  radio.setPALevel(RF24_PA_HIGH);

  //radio.openWritingPipe(pipe); // открываем трубу на передачу.
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();
}
////////////////////////////////////////////////////////////
boolean radiosend( int c)
{

  if (radio.write(&c, sizeof(c))) // отправляем данные и указываем сколько байт пакет
  {
    Serial.print("data: ");
    Serial.println(c);
  } else {
    Serial.print("Faild: ");
    Serial.println(c);
    return false;
  }
  return true;
}
//////////////////////////////////////////////////////////////////////
void radioping()
{
  int counter = 0;
  radio.setPayloadSize(sizeof(Radiodata));
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  for (byte i = 0; i < 10; i++)
  {
    lcd.print(">");
    radio.stopListening();
    Radiodata ping;
    ping.cmd=PINGCMD;   
    ping.data  = millis();
    bool ok = radio.write( &ping, sizeof(ping) );
    if (!ok)
    {
      Serial.println("write - faild");
    } else
    {
      radio.openReadingPipe(1, pipes[1]); // без этого не работает
      radio.startListening();
      // Wait here until we get a response, or timeout (250ms)
      unsigned long started_waiting_at = millis();
 
      bool timeout = false;
      while ( ! radio.available() && ! timeout )
        if (millis() - started_waiting_at > 200 )
          timeout = true;


      if ( timeout )
      {
        Serial.println("Failed, response timed out.");
      }
      else
      {
        // Grab the response, compare, and send to debugging spew
        Radiodata got_ping;
        radio.read( &got_ping, sizeof(got_ping) );
        // Spew it 
        unsigned long curtime=millis();
        if(got_ping.cmd == PINGANSWERCMD)
        {
         counter++;
          
         Serial.print("Got response: ");
         Serial.print(got_ping.data);
         Serial.print(" round-trip delay: ");     
         Serial.println(curtime - got_ping.data);
        }else{
          Serial.print("Unexpect recievd non ping answer code:");
          Serial.println(got_ping.cmd);
        }
         
      }
      delay(30);
    }
  }
  

 char strbuff[17] ;
 //dtostrf(counter/10*100, 2, 0, strbuff);

  sprintf(strbuff,"%d/%d packet sent",counter,10);
 // Serial.println(psent);
 // dtostrf(_val, 2, 1, str_buf);
 
  Serial.println(strbuff);
  lcd.setCursor(0, 1);
  lcd.print(strbuff);
  delay(1000);
}
//////////////
//////////////////////////////////
void usefulWork()
{
   float tDst=22.0; //Температура назначения
 
   //Сначала определим в каком режиме мы находимся - ручном или автомат
   if(TS.mode.val() == 0) // AUTO режим
   {
          RTCtime.gettime();
          int i=TS.findTermoPeriod(RTCtime.Hours,RTCtime.minutes); // определим в какаом термопериоде мы наход
          if(i<0){TS.relay_ctl(OFF); return;} //Выключаем реле если мы находимся вне всяких термопериодов       
          tDst = TS.termoPeriod[i].temp(); //Заданная температура периода
   }else //Ручной режим
   {
    // Если в ручном - определим граничную  температуру    
         tDst= TS.tempMan.val();
   }
    states cmd;    //Команда для реле
    switch (TS.relay) //Текущее состояние  реле 
    {
      case ON:
          cmd=ON;
          if(TS.temper>=tDst+TS.hyster.val())
          {
            cmd=OFF;
          }
          break;
      case OFF:
          cmd=OFF;
          if(TS.temper<=tDst)
          {
            cmd=ON;
          }
      
          break;
           
    }

    TS.relay_ctl(cmd); //Посылаем комнду в реле
}


void setClockTime()
{
  RTCtime.gettime();
  Time tm(RTCtime.Hours, RTCtime.minutes);
  lcd.clear();
  lcd.setCursor(0, 0); //инфо на LCD
  lcd.print(F("SET CLOCK TIME "));
  tm.setpos(5,1);
  if(tm.edit()==btnSELECT){
     RTCtime.settime(0,tm.m(),tm.h());
  }
  return;
}

void setClockDate()
{
  RTCtime.gettime();
  Date dt(RTCtime.day, RTCtime.month, RTCtime.year);
  lcd.clear();
  lcd.setCursor(0, 0); //инфо на LCD
  lcd.print(F("SET CLOCK DATE"));
  dt.setpos(3,1);
  if(dt.edit()==btnSELECT){
     RTCtime.settime(-1,-1,-1,dt.d(),dt.m(),dt.y());
  }
  return;
}










