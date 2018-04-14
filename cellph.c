#include "cellph.h"
//-------------------------------------------------------------
void send_sms(const unsigned char *recipient, const unsigned char *msg)
{
  if (NULL != recipient && NULL != msg) {
    unsigned char smslen = 0;
    unsigned char sms[200] = {0};
    // подготовить смс из текста и номера
    smslen = create_pdu(sms, recipient, msg);    
	
	_delay_ms(100);
	uart_puts("AT+CMGS=");
    uart_put_int(smslen);
    uart_puts("\r\n");
    
	_delay_ms(100);
    uart_puts(sms);

	//_delay_ms(5000);  
  }
  return;
}
//-------------------------------------------------------------
/*void dial_up(char *recipient) 
{
  if (NULL != recipient) {
    _delay_ms(2000);
	char cmd[32] = {0}; 
	strcpy(cmd, "ATD");
	strcat(cmd, recipient);
	strcat(cmd, ";\r\n");
	uart_puts(cmd);
	_delay_ms(10000); // wait for Response from cellphone
  } else {
    uart_puts("dial_up::NO RECIPIENT\r\n");
  }
  return;
}*/
//-------------------------------------------------------------
void hang_up(void) // может быть передавать режим 0..5 типы соединений 
{ 
  _delay_ms(2000); // get call status if ended then hang up
  uart_puts("ATH0\r\n");
  return;
}
