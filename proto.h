// ������ 0.5
// �������� ��������� asciictl.h
// ��������� ����������� ������ ������ ���������� �� ���
// ��������� ������� ������ ��������� ����� � ��� set soundtrack 
// �������� ����� ����� �����(STREAM)

#ifndef PROTO_H
#define PROTO_H
	  
	 /*
	   ��������!
	   �������� ������� ������ � ������ ����� 
	   �������� �������� ������������ uart_getc() 
	   �� ��������� NUL ��� (0x00) ��� ������������
	 */     
   
    
//Packet Format
// [] - optional parameter 
//     Byte 1     |    Byte 2       |       Byte 3         | Bytes 4 - PDL |    PDL + 1
//Start of Header | [Packet Number] | [Data Packet Lenght] |  Packet Data  |  End of Text
      /*
	  �������� v.2
      Request = SOH  <PN> <DPL> <PD> ETX
	  ��������� = ���������� ESTABLISH
	   PC: ENQ ->
	   		   <- ACK | timeout :Controller
      
	  ��������� = ����� ������� INTERACT
       PC: Request ->
	   			   <- Response | ACK | NAK | timeout :Controller				 
	  ��������� = ���������� ������ TERMINATE
       PC: EOT ->
	   		   <- ACK | timeout :Controller
	 
	 
	   � ��������� INTERACT
	   PC: DLE ->
	   		   <- CK :Controller
       ��������� = ���� ����� STREAM
	   packet = SOH Data[128] ETX
	   PC: packet ->
	   			  <- ACK :Controller
             ....
       ��������� �������� �����
	   PC: FS FF EOT ->
	                 <- ACK :Controller
       ��������� = INTERACT
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

// ��������� ���������
#define ESTABLISH 0
#define INTERACT 1
#define TERMINATE 2
#define STREAM 3

#define PACKET_SIZE 132

// ��������� �������
typedef struct {
	char *comand; // ��� ������� (������ � ���� ����������)
	char (*action)(char* packet_data, char* result); // ��������� �� ������� ����������� ������� ������������� �������
} conf_cmd_t;

typedef char packet_t;
typedef char packet_data_t;


// ������� ����������� �� ��������
char on_get_devinfo__act(packet_data_t*, packet_data_t*); 
char on_set_sms__act(packet_data_t*, packet_data_t*);
char on_set_phonebook__act(packet_data_t*, packet_data_t*);
char on_set_delay__act(packet_data_t*, packet_data_t*);
char on_get_sms__act(packet_data_t*, packet_data_t*);
char on_get_phonebook__act(packet_data_t*, packet_data_t*);
char on_get_delay__act(packet_data_t*, packet_data_t*);
char on_get_devstat__act(packet_data_t*, packet_data_t*);

// ������ ����������� ��������� ���������
#define wait_for_data(c) do { c = uart0_getc(); } while (!c);
//char wait_for_data(void);
void process_packet(packet_t* packet, conf_cmd_t* cc_list); 
void send_response(packet_t* packet, packet_data_t* data);
char create_packet(packet_t* packet, packet_data_t* data);
void receive(void);

// ���� ��������� ���� ��������� ��� �����������
//---------------
#define SMS_TEXTLEN_MAX 70
#define PHONEBOOKLEN_MAX 5
#define PHONELEN_MAX 20
//---------------


// ������ ������������� ������ � ��������������� �� ��������
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

// ��������� ��������������
static uint8_t proto_state = ESTABLISH;

static uint32_t proto_nv_addr = 0;
static uint32_t proto_packet_number = 1;

static uint8_t proto_start_rcv = 1;

#endif //PROTO_H
