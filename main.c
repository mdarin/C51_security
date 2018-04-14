/* * * * *
 * ������� ��� ���������� ������� ���������
 * 
 * ���� ��������: 21.10.2013 
 * ���� ���������: 18:34 30.04.2014
 * ������ 0.51rc
 *
 * 
 * ������ �������:
 *
 * ������ ���������� �� avr-gcc4.7.2
 *
 * ChangeLog:
 *
 * 0.51rc
 *  + ������ �������
 * 0.5rc
 *  + ������������ ���������
 *  * ������ ���������
 *  + �������� ���������� �� ������
 *    ������ ����� � ���������� 
 * 0.41b
 *  * �������� ���������� uart �� ����� ������
 *  * ������������� ��� ��� ���������� uart
 * 0.4b
 *  * ���������� ���������� ������� �� �������(� ���� ����������, �������������) �������� :)
 * 0.3b 
 *  + �������� ������� ��������� ��������� DVSR
 *  + �������� ���������� ������� ������ ��������� ��������� get dev stat(�������� �������� DVSR)
 *  + ����������� ���������� ������������� ��� ���������� 0 � 'empty'
 *  + ���������� ��������� �������(�������� ���������������� ������ � ����������, 
 *    ��������� ����� ������ � ��������� cellph.h/c
 *  * ����������� ������� ��������� �� ������ �� ��������(�� �����������) 
 * 0.2b
 *  + ��������� ��������(������ ���������)
 *  + �������� �������� �� ������ �������� ��������� �� ���(�� ��������!)
 *  * ������� ���������� ����� �������� �� ���������� get/set delay 
 * 0.1b
 *  + ���������� ������ � �������� �������
 *    ������ ����� � ����������
 *
 * TODO:
 * 1.������� �������� ��� ���������� �� ������ ������ �������� �� ������������ [+] 
 * 2.������� �������� ������ ������   [+] 
 * 3.������� ������� �� ������ ������, ����� �������� �� 
 * 4.����������� �������� ����� � ��������� (������ ������) [+]
 * 5.������� ������� ������ � ��������� � ���� cellph.h cellph.c [+]
 * 6.������� ������� ������ � ����� ���� � ���� handsfree.h [+]
 * 7.�������� ����� ������������� � ���������� �������� [+] ������� ������ ��������� DVSR
 * 8.����������� ��������� ������� :) ��, ����� �����
 * 9.����������� ���� �������� ��� ������� �������� �� ���������� [+]
 * 10.����������� ����� ��������� ������� ������ ������� � ��� ������� [+-]
 * 11.����������� ����� ��������� ������� �� ������� [-]
 * 12.���������� �������� ��� ������ �����, ��� �������������� ���������� sim-����� [-]
 * 13.����������� ����������� ������ ��������� ����� ���������������� ���������� � ��
 * 14.����������� ������ ����� ����� �������� ������� [-]
 * * */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
//#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "include/led.h"
#include "include/uart/uart.h"
#include "include/uart/uart_addon.h"
#include "include/at25df161.h" // ������� ����-������ :) ����� ���� � sound.h ???
#include "include/wbuf.h" // ������� ����-������ :) ����� ���� � sound.h ???
#include "proto.h"
#include "cellph.h"
#include "handsfree.h"
#include "at.h"

#ifndef INFINITY
#define INFINITY 1
#endif
#ifndef F_CPU
#define F_CPU 20000000L
#endif


/*
ATV0 - ������ ������ ������.
1 - ������ ��������� ����� � ���� (���� �������). ����� �� ���������.
0 - ������� ������ ���� ������.
� ������ ������ ������ ����� ������������ OK, ERROR � ������, � �� ������ ����� ������������ ���� ����������. ��� ������ OK �������� 0, � ������ ERROR 4.


*/


#define PWM_TOP 255
#define PWM_BOTTOM 4


#define LED_BLINK 2
#define LED_PWM 3
#define ON 1
#define OFF 0
#define UP 1
#define DOWN 0

#define BLOCKSZ PAGE_SIZE // �������� �� ������ ������
#define DATA_LEN 1872352 / 8 //�������� �� ������ ������


// ��������� ��������� �������� ������������ ������ ����������
#define IDLE 0
//   * -> idle.
//idle -> security.
#define SECURITY 1
//security -> alarm,
//         -> idle.
#define ALARM 2
//alarm -> idle.
#define CONFIG 3
//config -> .      

static unsigned char dev_state = 0;
static unsigned int baudrate = //19200;
							   //38400;
							   57600;
							   //115200;

//static 
struct WBUFFER *buf;

#define SMS_TEXTLEN_MAX 70
#define PHONEBOOKLEN_MAX 5
#define PHONELEN_MAX 20

// ����� ���, ���������� ����� 
// � �������� ����� ����������� �� ������
// �������� � ���� �����������
uint8_t EEMEM nv_sec_delay;
uint8_t EEMEM nv_msg_text[SMS_TEXTLEN_MAX];
uint8_t EEMEM nv_ph_book[PHONEBOOKLEN_MAX][PHONELEN_MAX];
// ������� ��������� ���������� DVSR
// ���������� ������ �����������! :)
#define EEIF 0 // !!������� �������� ���� FEEI ���� ��� :) ���� ������������� ���
#define CFGF 1 // ���� ��������� ��� ���������� ���������� ���������������� � ������ � ������ ???
#define GSMIF 2 // ���� ������������� ��������, ������� ��������� � �����(����������� ��� ������)
#define GSMFTF 3 // ���� ������ �������� ��������������� � �������� ������ ���� ������� �� ��������
// ... �� � ��� ����� ������������� ���� :)
//���      
// 0   EEIF        ���� ������������� ����� ��� ���������� ������ � EEPROM ���� ��� �� ������������������� 0 - ������������������� 1 - �� �������������������
// 1   Reserved
// 2   Reserved
// 3   Reserved
// 4   Reserved
// 5   Reserved
// 6   Reserved
// 7   Reserved  
uint8_t EEMEM DVSR;


volatile uint8_t led_pwm_val = 0;
volatile direction = UP;
#define DIODE_PWM_PRESCALER_MAX 10
volatile uint16_t diode_pwm_prescaler = 0; 
#define DIODE_PRESCALER_MAX 120
volatile uint16_t diode_prescaler = 0;
volatile uint8_t led_state = LED_BLINK;
volatile uint8_t red_led_state = OFF;


void nv_init(void);
void play(void);
void stop_playing(void);
void dgen_init(void);
void pwm_init(void);

void dial_up_ver_two(char *recipient);


//-------------------------------------------------------------
void nv_init(void)
{
  // ��������� ������ ��������� ���������
  eeprom_busy_wait();
  uint8_t dvsreg = eeprom_read_byte((const void*)&DVSR);
  // ���� ��� �� ������������������� �� �������� � ��� �������� �� ���������
  if (bit_is_set(dvsreg, EEIF)) {
    // ��������� ����� ��������� ��������� empty
	eeprom_busy_wait();
	eeprom_update_block("empty", nv_msg_text, 6);
     
    // ��������� ��� ������ ���������� ����� ����������� empty
    for (uint8_t i = 0; i < PHONEBOOKLEN_MAX; i++) {	
      eeprom_busy_wait();
	  eeprom_update_block("empty", nv_ph_book[i], 6); 
    }    

    // �������� �������� �������� ����� ����������� �� ������ ����� 0
	eeprom_busy_wait();
	eeprom_update_byte(&nv_sec_delay, 0);

    // ���������� ��� ������������� ���(������� ���������)
    dvsreg &= ~(1 << EEIF);
	// �������� ������ ��������� ���������� � ���
	eeprom_busy_wait(); 
    eeprom_update_byte(&DVSR, &dvsreg);
    
	// __DEBUG
	uart0_puts("WR 'empty'|0 to EEMEM\r\n");
  }
  
  return;
}

//-------------------------------------------------------------
void play(void)
{ 
  init_spi_master(); 
  pwm_init();
  dgen_init();    
  handsfree_uart_disable();  
  handsfree_init();
  _delay_ms(2000); // ����� :)
  handsfree_enable();

  char done = 0;
  unsigned long addr;
  buf = wbuf_init(PAGE_SIZE/2);
  if (NULL != buf ) { 
     // at the begining fillin two buffer atonce
     // ���������� ����� �� ���� ������
     addr = 0;
	 // ��������� ��� ������
	 while (SetWriteProtectedArea(NONE) == BUSY){} // Disable Software Protection (ensure that #WP is high)		
	 while (GetCharArray(addr, buf->unload, buf->length) != TRANSFER_COMPLETED){}
     buf->block++;
   } else {
     // ���� ������ �� ������� �� ��������������� �� ������
     // loop on memory alloc error  
	 red_led_on();
	 uart0_puts("main::MEM ALLOC ERR\r\n");
	 while (1) continue;
   }
   
   while (!done) {
   	 green_led_on();
     //green_led_on();
	 //red_led_off(); // ��� ���� ���� ��������� ��� ��� ��������� �� �������� ��� �����
	 asm("nop");
	 asm("nop");
	 asm("nop");
	 asm("nop");
	 asm("nop");
	 //-----------------------------------------
     // �������� ��������� ������
     if (empty == buf->status) {	    
	   // �������� ����� ���������� �����
	   addr = addr + buf->length;
	   // ������� ���� � �����
	   while (GetCharArray(addr, buf->reload, buf->length) != TRANSFER_COMPLETED){}				
	   buf->status = full;
	   buf->block++;  
	   // ��������� ������, ���� ��������� ����� ������
	   // ����� � ������ ���������� � ����� ���������� �����, � ������ ����,
	   // ���������������� �� ��������� ������	   
	   // dependendent part... need some mind force :)
	   if (addr >= DATA_LEN) {
	     stop_playing();
	     buf->block = 0;
		 done = 1;
	   } 
	 }
	 continue;
   }
   wbuf_destroy(buf);
   // ��������� ���� ����� ���� ��� �� ��� ������������������ ��� ������ ���������
   uart0_init(UART_BAUD_SELECT(baudrate, F_CPU));
   return;
}


void stop_playing(void)
{
  TCCR1B &= ~(1 << CS10);
  TCCR0B &= ~(1 << CS00);

  handsfree_disable();

  green_led_off();
  return; 
}


//-------------------------------------------------------------
void dgen_init(void)
{
  // T1 as 11-22 generator init 
  TIMSK1 |= (1 << OCIE1A);
  TCCR1B |= (1 << WGM12) | (1 << CS10);
  TCNT1 = 0;
  OCR1AH = 0x03; // 22 KHz
  OCR1AL = 0x8C;
  return;
}
//-------------------------------------------------------------
void pwm_init(void)
{
  // T0 as fast PWM on OC0B(PD5) pin
  //__DEBUG
  TIMSK0 = 0;

  TIMSK0 |= (1 << OCIE0B);
  TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
  TCCR0B |= (1 << CS00);  
  // PWM output ON
  DDRD |= (1 << PD5);
  return;
}
//-------------------------------------------------------------
ISR (TIMER0_COMPB_vect) 
{
  // continue...   
  return;
}
//-------------------------------------------------------------
ISR (TIMER1_COMPA_vect)
{
  wbuf_check_toggle(buf);
  OCR0B = wbuf_get_value(buf);
  buf = wbuf_get_next(buf);       	    
  return;
}
//-------------------------------------------------------------
ISR (TIMER0_OVF_vect)
{
  // ��������� ��� ������ ��������
  TCCR0B = 0;
  

  TCNT0 = 0;
  TCCR0B |= (1 << CS02) | (1 << CS00);
  return;
}

//-------------------------------------------------------------
/*��� �� �����������.
�� ��� ������ � ����� ��� ���������, ���������� ���, 
� ������ OCxx ��� ������������� ��� ������ ����.
 
������ ���������, ����� ������ ����� ��� ���������. 
����� ��������� ���, ������:
    ��������� ������ OCxx �� �������� ���������.
    ��������� ��� ����������� ���������� �� ��������� � �� ������������. 
	� ���������� �� ��������� ���������� ������ ���, � ���������� �� ������������ �������� �������������.

��� ������ :)*/

ISR (TIMER2_COMPB_vect)
{ 
  // ����������� �������� ��� ����������� ������� �������� ��� ����� ������
  if (DIODE_PWM_PRESCALER_MAX <= diode_pwm_prescaler) {
    switch (direction) {
    case UP:
      if (PWM_TOP <= ++led_pwm_val) direction = DOWN;
      break;
    case DOWN:
      if (PWM_BOTTOM >= --led_pwm_val) direction = UP;
	  break;
    //ATTENTION! ���� �������� � OCR �������� ������ ~4 �������� ��� !
	//led_pwm_val = 4;
	}
	
    diode_pwm_prescaler = 0; 
  }else {
    diode_pwm_prescaler++;
  }
  
  OCR2B = led_pwm_val;
  red_led_off();
  return;
}



ISR (TIMER2_OVF_vect)
{
  TCCR2B = 0;
  TCNT2 = 1;
  
  switch(led_state) {
  case LED_BLINK:
    // ����������� �������� ��� ����������� ������� �������� ��� ����� ������
    if (DIODE_PRESCALER_MAX <= diode_prescaler) {
      if (ON == red_led_state) {
        red_led_on();
	    red_led_state = OFF;
      } else {
        red_led_off();
	    red_led_state = ON;    
      }	
      diode_prescaler = 0; 
    }else {
      diode_prescaler++;
    }
    break;
  case LED_PWM:
    red_led_on();
    break;
  }
  TCCR2B |= (1 << CS22) | (1 << CS20);
  return;
}


void init_led_pwm(void)
{  
  TIMSK2 |= (1 << OCIE2B) | (1 << TOIE2);
  OCR2B = led_pwm_val;
  TCNT2 = 1;
  TCCR2A |= (1 << WGM20);
  TCCR2B |= (1 << CS22) | (1 << CS20);
  led_state = LED_PWM;
  led_pwm_val = PWM_BOTTOM;
  direction = UP;
  return;
}

void deinit_led_pwm(void)
{  
  TIMSK2 = 0;
  TCCR2A = 0;
  TCCR2B = 0;
  red_led_off();
  return;
}


//-------------------------------------------------------------
void init_led_blink(void)
{
  TIMSK2 |= (1 << TOIE2);
  TCNT2 = 0;
  TCCR2B |= (1 << CS22) | (1 << CS20); // prescaler 1024
  led_state = LED_BLINK;
  return;
}

void deinit_led_blink(void)
{  
  TIMSK2 = 0;
  TCCR2A = 0;
  TCCR2B = 0;
  red_led_off();
  return;
}

//-------------------------------------------------------------
void startup_init(void) 
{
  leds_init();
  uart0_init(UART_BAUD_SELECT(baudrate, F_CPU));
 
  sei();
  green_led_off();
  red_led_off(); 
  
  // ���� ���������� ��������� ������� � ��������� ������������ ��� ����� ���� � �� ����� :)
  if (bit_is_set(PINC, PINC2)) {
    _delay_ms(70);
    if (bit_is_set(PINC, PINC2)) {
      dev_state = CONFIG;
    }
  }

  // ���� ������ ��� ����� �� ������������������� � ������� ������ 0 � � ������ 'empty'
  nv_init();
  
  //init_spi_master();
  return;
}
//-------------------------------------------------------------
int main(void) {

  startup_init();
  


  char done = 0;
  
  // ��� ���������� ������ ����� ���� ������� � ������!!!
  //
  //
  char *recipient = NULL;
  char recipient_arr[PHONELEN_MAX] = {0};
  char *msg = NULL;
  char stop_wait_pick_up;
  char *cmd = NULL;
  static char dialed = 0;
  double balance;
  
  // �������� �� ��������� �������������
  _delay_ms(3000);
  uart0_puts("====|START|===\r\n");
 
  switch (dev_state) {
  case CONFIG:
    //uart_puts(" * -> [config]\r\n");

    break;
  default:
    dev_state = IDLE;

	// ��� ��� �������������, ���� ���
    // --------------------------------
    at_send_req("AT");
    //at_get_resp();
    _delay_ms(300);

    at_send_req("ATE0");
    at_get_resp();
    at_destroy_resp();

    _delay_ms(300);
    // �������� ������� ������� AT+CSQ 
	// �������� ������ ���������� �������� AT+CPAS 
	// �������� ���� � �����
	at_send_req("AT+CCLK?");
	at_get_resp();
	if (AT_RESP_OK == at_parse_resp()) {
      // �������� ��������� ������
	  char **resp_params = at_get_resp_params();
      // ���� ��������� ����, �� ���������������� ��������� ���������
	  if (NULL != resp_params) {
	    //for (uint8_t i = 0; i < at_get_resp_pc(); i++) {
	      uart_puts(":d:");
		  uart_puts(resp_params[1]);
		  uart_puts("\r\n");
		  uart_puts(":t:");
		  uart_puts(resp_params[2]);
		  uart_puts("\r\n");
		//}
	  }
    }
	at_destroy_resp();

    _delay_ms(300);
    // �������� ������� ������� AT+CSQ
	//<rssi>:
    //0:-113 dBm ��� ������
    //1:-111 dBm
    //2 �� 30:-109 �� 53 dBm
    //31:51 dBm ��� �����
	at_send_req("AT+CSQ");
	at_get_resp();
	if (AT_RESP_OK == at_parse_resp()) {
      // �������� ��������� ������
	  char **resp_params = at_get_resp_params();
      // ���� ��������� ����, �� ���������������� ��������� ���������
	  if (NULL != resp_params) {
	    //for (uint8_t i = 0; i < at_get_resp_pc(); i++) {
	      uart_puts(":r:");
		  uart_puts(resp_params[1]);
		  uart_puts(" dBm\r\n");
		//}
	  }
    }
	at_destroy_resp();


    _delay_ms(300);
    // �������� ������ ���������� �������� AT+CPAS 
	at_send_req("AT+CPAS");
	at_get_resp();
	if (AT_RESP_OK == at_parse_resp()) {
      // �������� ��������� ������
	  char **resp_params = at_get_resp_params();
      // ���� ��������� ����, �� ���������������� ��������� ���������
	  if (NULL != resp_params) {
	    //for (uint8_t i = 0; i < at_get_resp_pc(); i++) {
	      uart_puts(":s:");
		  uart_puts(resp_params[1]);
		  uart_puts("\r\n");
		//}
	  }
    }
	at_destroy_resp();


    // ���������� ������� �������: AT+WBCM �� ���� �� ����...
    _delay_ms(300);
	at_send_req("AT+CBC");
	at_get_resp();
	if (AT_RESP_OK == at_parse_resp()) {
      // �������� ��������� ������
	  char **resp_params = at_get_resp_params();
      // ���� ��������� ����, �� ���������������� ��������� ���������
	  if (NULL != resp_params) {
	    for (uint8_t i = 0; i < at_get_resp_pc(); i++) {	      
		  uart_puts(":b:");
		  uart_puts(resp_params[i]);
		  uart_puts("\r\n");
		}
	  }
    }
	at_destroy_resp();


   // ��������� ������, ��� �������� �� ����� ������� ������ ���� ��� �������...
   /*_delay_ms(300);
	at_send_req("ATD*105#;");
	at_get_resp();
	if (AT_RESP_OK == at_parse_resp()) {
      // �������� ��������� ������
	  char **resp_params = at_get_resp_params();
      // ���� ��������� ����, �� ���������������� ��������� ���������
	  if (NULL != resp_params) {
	    for (uint8_t i = 0; i < at_get_resp_pc(); i++) {	      		  
		  uart_puts(resp_params[i]);
		  uart_puts("\r\n");
		}
	  }
    }
	at_destroy_resp();
    */
   /* _delay_ms(300);
	balance = at_get_balance("102");
    uart_puts("::bal: ");
	uart_put_int((int)balance);
    uart_puts("\r\n");
    
	if ((int)balance < 0) {
	  uart0_puts("::balance less then 1 rub.\r\n");    
	}
    */
	
	//--------------------------------
	 
    uart0_puts(" * -> [IDLE]\r\n");
    break;
  }

  // �������� ���� ����������� ������ ������ ����������
  while (INFINITY) {  
    switch (dev_state) {
	case CONFIG: // ----------------------------------
      deinit_led_pwm();
	  deinit_led_blink();
	  // �������� �������
	  if (bit_is_clear(PINC, PINC2)) {
	    _delay_ms(70);
	    if (bit_is_clear(PINC, PINC2)) {          
          dev_state = IDLE;
		  uart0_puts("[CONFIG] -> [IDLE]\r\n");
	      init_led_blink();
        }
      }
	  // ����������� ���������������� �������������� � ��
	  receive();
 	  break;
	case SECURITY: // --------------------------------
	  // �������� �������
	  if (bit_is_set(PINC, PINC1)) {
	    _delay_ms(70);
	    if (bit_is_set(PINC, PINC1)) {
          uart0_puts("[SECURITY] -> [IDLE]\r\n");
          deinit_led_pwm();
		  dev_state = IDLE;
        }
      } else if (bit_is_set(PINC, PINC0)) {
	    _delay_ms(70);
	    if (bit_is_set(PINC, PINC0)) {
          uart0_puts("[SECURITY] -> [ALARM]\r\n");
          dev_state = ALARM;
          deinit_led_pwm();
	      init_led_blink();
        }
      }
	  // ============================
	  // �������� ���������� ��������
      // ������� � �������� ����!!!	   
      // ============================
	  // ����������� �������� �� ������
	  // secure(); 
	  break;
    case ALARM: // ---------------------------------
	  // ����������� �������� ��� ��������
	  // alarm();
	  done = 0;
      
	  msg = malloc(sizeof *msg * SMS_TEXTLEN_MAX);
	  // ���� ������ �� ������� �� ��������������� �� ������
	  if (NULL == msg) {
		// loop on memory alloc error  
	    red_led_on();
	    uart_puts("main::MEM ALLOC ERR\r\n");
	    while (1) continue;
	  }
      
	  // ������� ��������� �� ���
      eeprom_busy_wait();
      eeprom_read_block(msg, &nv_msg_text, SMS_TEXTLEN_MAX);
      // ��������� �� ������ ����� ��������� � ������ ��� �� ��������� ���� �� �� �����
      if (!strcmp(msg, "empty")) {
	    strcpy(msg, "����� ��������� �� �����");
	  }      

	  // ��������� ��� ��������� ���� ���������
	  for (char i = 0; i < PHONEBOOKLEN_MAX; i++) {
        
		recipient = malloc(sizeof *recipient * PHONELEN_MAX);
		if (NULL == recipient) {
          red_led_on();
	      uart0_puts("main::MEM ALLOC ERR\r\n");
	      while (1) continue;
		} 
			
		// ������� �������� �� ���������� �����(���)
		eeprom_busy_wait();
        eeprom_read_block(recipient, &nv_ph_book[i], PHONELEN_MAX);
		// ���������� ������ ���������
		if (!strcmp(recipient, "empty")) {
		  free(recipient);
		  continue;
		}
		_delay_ms(1000);
        send_sms(recipient, msg);
		at_get_resp();
		at_destroy_resp();
        
		//uart_puts("::sms to:");
		//uart_puts(recipient);
		//uart_puts("\r\n");
		//uart_puts(msg);
		//uart_puts("\r\n");
		//_delay_ms(500);

		free(recipient);
		
		_delay_ms(3000); 
      }
      free(msg);
      
	  
	  
	  // ������������� ��������� ��� ����������� ����� ������� �������
	  // ��� �������
	  // ������� �������� ������ �������� � ������ ������, ���� ������� 
	  // �� ������ ��������� ����� � ������ �������
	  // ����, ���� ���������� ����, �� �������� � �������� ������... �� �����
	  //at_send_req("ATH0");
      //at_get_resp();
	  // ����� �������������� ������ �� ������ ����������
	  // �������� �����������
	  //while(AT_RESP_OK != 
	  //at_parse_resp();
	  //) continue;
	  //at_destroy_resp();
	  //_delay_ms(2000);
	  
	  //	at_get_resp();
	//	at_destroy_resp();
      _delay_ms(2000);

      // ���������!
	  // ��� ���� � ����������� ���� ��� ������� �������� ���������� �� ������� �������(��������) �� ������ �������� ������ �����, � ��������� ���� �� ������� � �������� ���� �� �������� NO DIALTONE
	  // ���� ��������� ������� ���������� �������, �������� ������ ��������� �� ��� ���...
	  // ���� ��������� ���� ����� ����� ��� � ������ ������ �����, ������ ���-��������� ���� �����������

	  // ��������� ���� ���������    
	  for (char i = 0; i < PHONEBOOKLEN_MAX; i++) {
        
		//uart0_puts("::calling...\r\n");
		recipient = malloc(sizeof *recipient * PHONELEN_MAX);
		if (NULL == recipient_arr) {
          red_led_on();
	      uart0_puts("main::MEM ALLOC ERR\r\n");
	      while (1) continue;
		}

        //char recipient[PHONELEN_MAX] = {0};
		
        //void * memset ( void * ptr, int value, size_t num );
        //memset(recipient_arr, 0, PHONELEN_MAX);
		// ������� �������� �� ���������� �����(���)
		eeprom_busy_wait();
        eeprom_read_block(recipient, &nv_ph_book[i], PHONELEN_MAX);
		// ���������� ������ ���������
		if (!strcmp(recipient, "empty")) {
		  free(recipient);
		  //uart0_puts("::skip\r\n");		 
		  continue;
		}		
		
		//
	    // dial_up ����������� �� ���� ������� � �������...
	    //	     	     
		// �������� ���� �������� ��������� ����������
		dialed = 0; 
	
		// ����������� ������� ������� ��� �������� ������ �������
		cmd = malloc(sizeof *cmd * strlen(recipient) + 1 + 4);
	    if (NULL == cmd) {
          red_led_on();
	      uart0_puts("main::MEM ALLOC ERR\r\n");
	      while (1) continue;
	    }
		sprintf(cmd, "ATD%s;", recipient);
		
		//__DEBUG
		//uart_puts(recipient);
		//uart_puts("\r\n");
		


		_delay_ms(2000);
        
		// ������� ����� �������� ��������       	 
		at_send_req(cmd);        		
	
        free(cmd);
		free(recipient); 

        //continue;
	    // ����� ����� �� ������� �������
		at_get_resp();
        // ���������������� �����, ���� �� �� ����� ������ ������
	    if (AT_RESP_OK == at_parse_resp()) {
		  // �������� �����	    
		  at_destroy_resp();
		  		  
		  // ����� ���� �������� ������	
	      stop_wait_pick_up = 0;

		  // ����� ������ ������ ��� ������ ������� �������� �������� ����������
		  while (!stop_wait_pick_up) { 
		   
		    // �������� ������ ��������
			_delay_ms(200);
		    
			at_send_req("AT+CLCC");
		    at_get_resp();

            // ���������������� ����� �� ������ ��������� ���������� 
		    if (AT_RESP_OK == at_parse_resp()) {
              // �������� ��������� ������
			  char **resp_params = at_get_resp_params();
              // ���� ��������� ����, �� ���������������� ��������� ���������
			  if (NULL != resp_params) {
	    
		    	//__DEBUG
				//uart_puts("\r\n:S:[");
				//uart_puts(resp_params[3]);
				//uart_puts("]\r\n");
        
				// ���� ������ ����� �� ��������� ��������
		        if (!strcmp(resp_params[3], "0")) {
		          stop_wait_pick_up = 1;
			   	 // uart0_puts("::is connected\r\n");
				  // ���������� ���� ��������� ����������
				  dialed = 1;
			    } 
			  } //eof if
               else { // ���� ���������� ��� ������ ������ �� ����� � ����� ������ ���� ����������
               // �����������
		       // ������� �������� ���� � ����������� ���� ������...
			   // ������ ����� ������ ������� =)
                stop_wait_pick_up = 1;        
		      }



              // ���� ������ ������ ������ ��������� ��������
			  if (at_is_busy()) {
			    stop_wait_pick_up = 1;
				uart0_puts("::is busy\r\n");
			  }

              // ���� ������ ������ ��� ������� ��������� ��������
              if (at_is_no_carrier()) {
			    stop_wait_pick_up = 1;
				uart0_puts("::is no carr\r\n");
			  }

			  // ���� �� ������� ������� ������������� �������� ������(���� �����)
			  if (at_is_no_dialtone()) {
			    stop_wait_pick_up = 1;
				uart0_puts("::is no dtn\r\n");
			  }

	         } // eof if AT_RESP_OK
          
		     // �������� �����
	         //_delay_ms(200);
		     at_destroy_resp();
	       }

		 }  //eof ATD if

		 //__DEBUG
		 //uart0_puts("::dial end:[");
		 //uart_put_int(dialed);
		 //uart0_puts("]\r\n");
	     //----------------

         if (dialed) {
	         //uart0_puts("::play\r\n");
			 //_delay_ms(150); // ����� ����� ���� ������...
			 play();
		     // �������� �� ������ ����� � �� ����� �����...
			 //_delay_ms(1000);
			 //
			 // hang up ����������� �� ���� ������� � �������...
			 //
			 _delay_ms(2000);
		     at_send_req("ATH0");
		     at_get_resp();
			 // ����� �������������� ������ �� ������ ����������
			 // �������� �����������
			 while(AT_RESP_OK != 
			 at_parse_resp()//;
			 ) continue;
			 at_destroy_resp();
			 _delay_ms(2000);
		 } 

      }	//eof for        

      
	  // ��� ��� ������ ������ � �������� ������������ ��������
	  // ���� ��� ����� ������������ ����� ������� 5 ������ ��������
	  // �� ��������!
	  //uart0_puts("::canceling alarm...\r\n"); 
	  //_delay_ms(5000);      
  	       
	  // ����� ������ � ������	  
	  uart0_puts("::alarm done\r\n");
	  while (!done) {
	    //uart_puts("[alarm done]\r\n");
	    if (bit_is_set(PINC, PINC1)) {
	      _delay_ms(70);
	      if (bit_is_set(PINC, PINC1)) {
            dev_state = IDLE;
			done = 1;
			uart0_puts("[ALARM] -> [IDLE]\r\n");
			//__DEBUG
	        deinit_led_blink();
          }
        }
	  }
      
	  break;
    default: // IDLE --------------------------------
	  // �������� �������
	  if (bit_is_clear(PINC, PINC1)) {
	    _delay_ms(70);
	    if (bit_is_clear(PINC, PINC1)) {		  
          dev_state = SECURITY;
          uart_puts("[IDLE] -> [SECURITY]\r\n"); 
		  // ������� �������� (n * ���.) �� ���������� ���� ��� ������
		  unsigned int arm_delay = 0;
          eeprom_busy_wait();
	      arm_delay = eeprom_read_byte(&nv_sec_delay);          
		  if (arm_delay > 0) {
			//__DEBUG
			uart_puts("::delay:");
			uart_put_int(arm_delay);
			uart_puts("\r\n");
			for (uint8_t i = 1; i <= arm_delay; i++) {
			  _delay_ms(1000);
			}
		  }
		  init_led_pwm();
        }
		// �������� �������
      } else if (bit_is_set(PINC, PINC2)) {
	    _delay_ms(70);
	    if (bit_is_set(PINC, PINC2)) {
          dev_state = CONFIG;
		  uart_puts("[IDLE] -> [CONFIG]\r\n");
        }
      }
      // �������� ���������� �������� � ����� ������� � �������� ���� � �� ������� ������
       
	  break;
	}
     
     //continue;
  } // eof INNFINITY :)

  return 0;
}
