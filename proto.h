// версия 0.5
// добавлен заголовок asciictl.h
// добавлены обработчики пустых данных вычитанных из ПЗУ
// добавлена команда записи звукового файла в ПЗУ set soundtrack 
// добавлен режим приёма файла(STREAM)

#ifndef PROTO_H
#define PROTO_H
	  
	 /*
	   ВНИМАНИЕ!
	   проверть наличие данных в канале можно 
	   проверяя значение возвращаемое uart_getc() 
	   на равенство NUL или (0x00) что эквивалентно
	 */     
   
    
//Packet Format
// [] - optional parameter 
//     Byte 1     |    Byte 2       |       Byte 3         | Bytes 4 - PDL |    PDL + 1
//Start of Header | [Packet Number] | [Data Packet Lenght] |  Packet Data  |  End of Text
      /*
	  Протокол v.2
      Request = SOH  <PN> <DPL> <PD> ETX
	  состояние = соединение ESTABLISH
	   PC: ENQ ->
	   		   <- ACK | timeout :Controller
      
	  состояние = обмен данными INTERACT
       PC: Request ->
	   			   <- Response | ACK | NAK | timeout :Controller				 
	  состояние = завершение сеанса TERMINATE
       PC: EOT ->
	   		   <- ACK | timeout :Controller
	 
	 
	   в состоянии INTERACT
	   PC: DLE ->
	   		   <- CK :Controller
       состояние = приём файла STREAM
	   packet = SOH Data[128] ETX
	   PC: packet ->
	   			  <- ACK :Controller
             ....
       окончаине передачи файла
	   PC: FS FF EOT ->
	                 <- ACK :Controller
       состояние = INTERACT
	 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "include/uart/uart.h"
#include "include/uart/uart_addon.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "include/led.h"
#include "include/at25df161.h"
#include "devinfo.h"
#include "asciictl.h"

// состояния протокола
#define ESTABLISH 0
#define INTERACT 1
#define TERMINATE 2
#define STREAM 3

#define PACKET_SIZE 132

// структура команды
typedef struct {
	char *comand; // имя команды (строка в этой реализации)
	char (*action)(char* packet_data, char* result); // указатель на функцию реализующую дейсвие соответсующее команде
} conf_cmd_t;

typedef char packet_t;
typedef char packet_data_t;


// дейсвия выполняемые по командам
char on_get_devinfo__act(packet_data_t*, packet_data_t*); 
char on_set_sms__act(packet_data_t*, packet_data_t*);
char on_set_phonebook__act(packet_data_t*, packet_data_t*);
char on_set_delay__act(packet_data_t*, packet_data_t*);
char on_get_sms__act(packet_data_t*, packet_data_t*);
char on_get_phonebook__act(packet_data_t*, packet_data_t*);
char on_get_delay__act(packet_data_t*, packet_data_t*);
char on_get_devstat__act(packet_data_t*, packet_data_t*);

// функци реализующие транспорт протокола
#define wait_for_data(c) do { c = uart0_getc(); } while (!c);
//char wait_for_data(void);
void process_packet(packet_t* packet, conf_cmd_t* cc_list); 
void send_response(packet_t* packet, packet_data_t* data);
char create_packet(packet_t* packet, packet_data_t* data);
void receive(void);

// Надо придумать куда поместить эти определения
//---------------
#define SMS_TEXTLEN_MAX 70
#define PHONEBOOKLEN_MAX 5
#define PHONELEN_MAX 20
//---------------


// список обрабатывамых команд и соответствующих им действий
static conf_cmd_t cc_list[] = {	
	// GET actions
	{"get dev info", on_get_devinfo__act}, 
	{"get sms", on_get_sms__act},
	{"get phone book", on_get_phonebook__act},
    {"get delay", on_get_delay__act},
	{"get dev stat", on_get_devstat__act},
	// SET actions
	{"set sms", on_set_sms__act},
	{"set phone book", on_set_phonebook__act},
	{"set delay", on_set_delay__act},
	{NULL, NULL},// end of list
};



extern uint8_t nv_msg_text[SMS_TEXTLEN_MAX];
extern uint8_t nv_ph_book[PHONEBOOKLEN_MAX][PHONELEN_MAX];
extern uint8_t nv_sec_delay;
extern uint8_t DVSR;

// состояние взаимодействия
static uint8_t proto_state = ESTABLISH;

static uint32_t proto_nv_addr = 0;
static uint32_t proto_packet_number = 1;

static uint8_t proto_start_rcv = 1;

#endif //PROTO_H
