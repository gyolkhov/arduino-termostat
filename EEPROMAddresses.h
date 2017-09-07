#ifndef _EEPROMADDRESSES_H_
#define _EEPROMADDRESSES_H_
// EEPROM addresses
#define _MAGIC_WORD_ADDR 0 // byte По значению в этом поле определяем была ли инициализация памяти  значением 111 
#define _TEMPMAN_ADDR 1 // EE - адрес для сохранения температуры термостатирования(при ручном режиме), 4 байта(float)!
#define _TEMPCORR_ADDR 5 // EE - адрес для сохранения коррекции температуры, 4 байта(float)!
#define _HYSTER_ADDR 9 // EE - адрес для сохранения гистерезиса, 4 байта(float)!
#define _TERMO_PR1_ADDR 13 // EE - адрес для сохранения часов начала периода1  9 (byte)

#define _TERMO_PR2_ADDR 22 // EE - адрес для сохранения часов начала периода2  9 (byte)

#define _TERMO_PR3_ADDR 31 // EE - адрес для сохранения часов начала периода3  9 (byte)

#define _TERMO_PR4_ADDR 40 // EE - адрес для сохранения часов начала периода4  9 (byte)

#define _BEEP_ADDR 49 // EE - адрес для сохранения признака разрешения звука (byte)
#define _TEMPALARM_ADDR 50 // EE - адрес для сохранения значения недопустимого снижения температуры, 4 байта(float)!
#define  _MODE_ADDR 54 //EE - адрес для режима работы термостата AUTO/MAN

#endif
