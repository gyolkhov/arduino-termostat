#include "Buttons.h"
extern volatile byte MenuTimeoutTimer; //Глобальная переменная определенная в другом файле
extern uint8_t MenuTimeout ; //Глобальная переменная определенная в другом файле
static int keymap(int k) {
  if (k < 50) return btnRIGHT;
  if (k < 195) return btnUP;
  if (k < 380) return btnDOWN;
  if (k < 555) return btnLEFT;
  if (k < 790) return btnSELECT;

  return btnNONE;

}

/////////////////////////////////////////////////////////////////////////////
int read_buttons() //
{
  static unsigned long pr_btn_time = 0;
  int key, key2;
  unsigned long curtime = millis();
  do {
    key = analogRead(0);
    key = keymap(key);
    key2 = analogRead(0);
    key2 = keymap(key2);
  } while (key != key2);

  if ((key != btnNONE) && ((curtime - pr_btn_time) > 200)) {
    pr_btn_time = curtime;
    MenuTimeoutTimer = MenuTimeout;
    return key;
  }
  return btnNONE;

}
