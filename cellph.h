#ifndef CELLPHONE_H
#define CELLPHONE_H
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
//#include <avr/pgmspace.h>
//#include <avr/eeprom.h>
#include <util/delay.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <stdint.h>
#include "include/led.h"
#include "include/uart/uart.h"
#include "include/uart/uart_addon.h"
//#include "include/at25df161.h"
//#include "include/wbuf.h"
// новые заголовки
//#include "proto.h"
#include "sms.h"

void send_sms(const unsigned char *num, const unsigned char *msg);
//void dial_up(char *recipient) ;
void hang_up(void); // может быть передавать режим 0..5 типы соединений



 
#endif //CELLPHONE_H
