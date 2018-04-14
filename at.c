/* * * * * *  
 *  at.c - source for at interface lib
 *
 *  AT - AT interface Library
 *
 *  Copyright (C) 2014  Michael DARIN/������ ������
 *                      
 *                      
 *
 *  This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General
 *  Public License as published by the Free Software
 *  Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A
 *  PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General
 *  Public License along with this program; if not, write
 *  to the Free Software Foundation, Inc., 675 Mass Ave,
 *  Cambridge, MA 02139, USA.
 *
 *  Michael Darin may be contacted by email at:
 *     m-dot-darin-at-email-dot-su
 *
 * * */
//  ============== AT interface =============
//  
//
//
#include "at.h"
//-------------------------------------------------------------
void at_init_resp(void) 
{ 
  at_resp_count = 0;
  at_resp_pc = 0;
  at_clear_atd_stat_reg();
  return;
}
//-------------------------------------------------------------

//-------------------------------------------------------------

//-------------------------------------------------------------
char **at_get_resp_params(void)
{
  char **p = NULL;
  if (at_get_resp_pc() > 0) {
    p = at_resp_params;
	//__DEBUG
	//for (uint8_t i = 0; i< at_get_resp_pc(); i++) {
	//  uart_puts("$:");
	//  uart_puts(at_resp_params[i]);
	//  uart_puts("\r\n");
	//}
  }
  return p;
}

//-------------------------------------------------------------
/*void at_destroy_resp_params(void) 
{
  for (uint8_t i = 0; i < at_get_resp_pc() && i < AT_RESP_STACK_DEPTH; i++) {
	free(at_resp_params[i]);
	at_resp_params[i] = NULL;
  }
  return;
}*/
//-------------------------------------------------------------
void at_destroy_resp(void) 
{
  for (uint8_t i = 0; i < at_get_resp_count() && i < AT_RESP_STACK_DEPTH; i++) {
	free(at_response[i]);
	//at_response[i] = NULL;
  }

  //for (uint8_t i = 0; i < at_get_resp_pc() && i < AT_RESP_STACK_DEPTH; i++) {
    //free(at_resp_params[i]);
//	at_resp_params[i] = NULL;
  //}

  at_resp_count = 0;
  at_resp_pc = 0;
  at_clear_atd_stat_reg();
  return;
}

//-------------------------------------------------------------
void at_send_req(char *at_cmd)
{
  if (NULL != at_cmd) {
    //sprintf();
	char at_request[32] = {0};
	sprintf(at_request, "%s\r", at_cmd); 
	uart0_puts(at_request);
  }
  return;
}
//-------------------------------------------------------------
//
// return : count of at response tokens or 0 
uint8_t at_get_resp(void) 
{
  #define RESP_RESPONSE 'R'
  #define RESP_DELIM 'D'
  #define RESP_CMD_PROMPT 'P'
  
  //char data[256];// = "ATD+79615244722;\r\n";  
	//DWORD dwSize = sizeof(data);   
	//DWORD dwBytesWritten;    
  char c;
  
  uint8_t done;
  char parse_state = RESP_DELIM;
  uint8_t pos;
  char token[/*128*/32] = {0};
  //char buf[128] = {0};
  //uint8_t nresps = 1;
  uint8_t resp_received = 0;
  //uint8_t 
  // ���������� ���������� 
  at_resp_count = 0;
  // ��������� ������-��������� �� ����� �� ����� �������� ������ ��-����������
  at_timeout = 0; 

  //at_clear_resp_timeout_flag();
  //at_timeout_start();  
 
  while (!resp_received /*&& !at_timeout*/) {
    // get an at comand
    done = 0;
    pos = 0;
    
    while (!done) {
      //ReadFile(hSerial, &sReceivedChar, 1, &iSize, 0);
	  green_led_off();
	  do {
	    green_led_on();
		c = uart0_getc();		
      } while (!c/* && !at_timeout*/); // ����� �������
	  green_led_off();
	    
      switch (parse_state)  {  
      case RESP_CMD_PROMPT:
	    //uart_puts("state [promt]\r\n");
		//red_led_on();
        //while(1) continue;
		if ('\r' == c) {
        //_tprintf("[r]");
          parse_state = RESP_DELIM;
		  continue;
        } //else if ('\n' == c) {
      //_tprintf("[n]\n");
          //parse_state = RESP_RESPONSE;
        //}
	    break;
	  case RESP_DELIM:
        //uart_puts("state [delim]\r\n");
		//_tprintf("state [delim]\n");
		if ('\r' == c) {
        //_tprintf("[r]");
          continue;
        } else if ('\n' == c) {
      //_tprintf("[n]\n");
          parse_state = RESP_RESPONSE;
        } //else if ('>' == c) {
		  // ���� ����� ���� ��� � �� �� ������� ���... ���������� ��� ���� ������� �������� ����...
		  //uart0_puts("::[>]\r\n");
		  //parse_state = RESP_CMD_PROMPT;
		//}
        break;
      case RESP_RESPONSE:
      //uart_puts("state [response]\r\n");
	  //_tprintf("state [response]\n");
        if ('>' == c) {
		  uart0_puts("*[>]\r\n");
		  parse_state = RESP_CMD_PROMPT;
		  continue;
		} else 
		if ('\r' == c) {
        //_tprintf("[r]");
          continue;
        } else if ('\n' == c) {
        //_tprintf("[n]\n");
          token[pos] = '\0';
        //_tprintf(">%s\n", token);
          //__DEBUG
		  //uart_puts("::tok:[");
          //uart_puts(token);
		  //uart_puts("]\r\n");
		  
		  at_response[at_resp_count] = malloc(sizeof *at_response[at_resp_count] * strlen(token)+1);

		  if (NULL != at_response[at_resp_count]) {
            strcpy(at_response[at_resp_count], token);
		  } else {
            at_response[at_resp_count] = NULL;
		  }
        
		  at_resp_count++;   
		  		             
		  // ��������� ���� �������� ���������� ������
          if (!strcmp(token, "OK")) {
            resp_received = 1;
          } else if (!strcmp(token, "ERROR")) {
            resp_received = 1;
		  } else if (NULL != strstr(token, "+CME")) {
            resp_received = 1;
		  } else if (NULL != strstr(token, "+CMS")) {
            resp_received = 1;
		  } 

          done = 1; 
          pos = 0;
          strcpy(token, "");
          parse_state = RESP_DELIM;
        } else {
          token[pos++] = c;
        }
        break;
      } // eof switch
    } // eof accum token while
  } // eof receiving resp while

  //__DEBUG
  uart_puts("\r\nstack:\r\n");
  for (uint8_t i = 0; i < at_resp_count; i++) { 
    uart_puts("\r\n *|");
	uart_puts(at_response[i]);
	uart_puts("|\r\n");
  }
   
  return at_resp_count;
}
//-------------------------------------------------------------
/*
�������������� ������ � �������������� ����
������ ���������� � ������������� ��������� 
<CR><LF>(����� ���-���� ������ ATV0 DCE) � ATQ1(���������� ��������������� ����).
����� ��������� ������� �������, �� �������� �ERROR�.
� ���� ��������� ������� �����, �� ��� ���� ��� ������� � ��������� 
�����������, �� �������� ������ +CME  ERROR: <Err> ��� +CMS 
ERROR: <SmsErr> �  ����������������  ������  ������,  ����  �� 
����� CMEE ���� ��������� �������� 1. �� ���������, �������� CMEE
���������� 0, � ��������� �� ������ �������� ������ ��� �ERROR�.
� ���� ������������������ ������ ���� ��������� �������, �� �������� 
��ʻ.
� ��������� �������, ��������, ��� AT+CPIN?
��� ������������ ��-������������� ����������, ����� �� ������ ��� �������� ������. 
� ������-��� �������� <CR> � <CR><LF>��������� �������.
*/
// return PARSE_ERR if malformed list of response velues prpcessed
//        OK 
//        ERROR
//         
uint8_t at_parse_resp(void) 
{ 
  uint8_t status = AT_RESP_ERROR;
  if (at_resp_count > 0) {
    uint8_t done = 0;
	char *token;
#define AT_DELIMITERS "\";, \t"
	char *delims = AT_DELIMITERS;
	uint8_t at_resp_pos = 0;
	// ���������� ����������
	at_resp_pc = 0;
	
	//__DEBUG
	//uart_puts("\r\npars:\r\n");
	 
    // ���������� ������ ����� ����� �� ������
    while (!done && at_resp_pos < at_resp_count /*&& at_resp_pos < AT_RESP_STACK_DEPTH*/) {

	  if (!strcmp(at_response[at_resp_pos], "OK")) {
		  // ��� ���� ������� 0 ����� ����� ��� �� ������
		  //__DEBUG
		  //uart_puts("ok::[");
		  //uart_puts(at_response[at_resp_pos]);
		  //uart_puts("]");
		  status = AT_RESP_OK;
		} else if (!strcmp(at_response[at_resp_pos], "ERROR")) {
	      //__DEBUG
		  //uart_puts("err::[");
		  //uart_puts(at_response[at_resp_pos]);
		  //uart_puts("]");
		  status = AT_RESP_ERROR;
		} /*else if (!strcmp(token, "CME")) {
          // ��� ���� ������� ��� ������ CME
		} else if (!strcmp(token, "CMS")) {
		  // ��� ���� ������� ��� ������ CMS
		} */		
		else if (!strcmp(at_response[at_resp_pos], "BUSY")) {		    
		  //__DEBUG
		  uart_puts("busy::[");
		  uart_puts(at_response[at_resp_pos]);
		  uart_puts("]");
		  atd_resp_stat_reg |= (1 << AT_BUSY);
		  //return AT_RESP_ERROR;            
		} else if (!strcmp(at_response[at_resp_pos], "NO ANSWER")) {
          //__DEBUG
		  uart_puts("no ans::[");
		  uart_puts(at_response[at_resp_pos]);
		  uart_puts("]");
		  atd_resp_stat_reg |= (1 << AT_NO_ANSWER);
		  //return AT_RESP_ERROR;			
		} else if (!strcmp(at_response[at_resp_pos], "NO CARRIER")) {
          //__DEBUG
		  uart_puts("no car::[");
		  uart_puts(at_response[at_resp_pos]);
		  uart_puts("]");
		  atd_resp_stat_reg |= (1 << AT_NO_CARRIER);
		  //return AT_RESP_ERROR;
        } else if (!strcmp(at_response[at_resp_pos], "NO DIALTONE")) {
          uart_puts("no dt::[");
		  uart_puts(at_response[at_resp_pos]);
		  uart_puts("]");
		  atd_resp_stat_reg |= (1 << AT_NO_DIALTONE);
		} else if (!strcmp(at_response[at_resp_pos], "RING")) {
          //uart_puts("no dt::[");
		  //uart_puts(at_response[at_resp_pos]);
		  //uart_puts("]");
		  //atd_resp_stat_reg |= (1 << AT_RING);
		} else {
		   // �������� ��������� ������
		   // �������� ������ ����� ��� �������
	       token = strtok(at_response[at_resp_pos], delims);	  
	       while (NULL != token) {
	    
		    //__DEBUG
			//uart_puts("tok::[");
		    //uart_puts(token);
		    //uart_puts("]\r\n");
		    
			at_resp_params[at_resp_pc++] = token;
			
        //__DEBUG
			//uart0_puts("\r\n::[");
		   //uart0_puts(token);
		    //uart0_puts("]\r\n"); 

           // �������� ��������� �����
		   token = strtok(NULL, delims);
	      }
	  }
	  //uart_puts("\r\n");
	  at_resp_pos++;
	}
  }
  return status;// = OK|ERROR|CME_<err>|CMS_<erro>.
}

//-------------------------------------------------------------
void at_rise_resp_timeout_flag(void)
{
  //at_wait_resp_timeout = 1;
  return;
}
//-------------------------------------------------------------
void at_clear_resp_timeout_flag(void)
{
  //at_wait_resp_timeout = 0;
  return;
}
//-------------------------------------------------------------
uint8_t at_get_resp_count(void)
{
  return at_resp_count;
}
//-------------------------------------------------------------
uint8_t at_get_resp_pc(void)
{
  return at_resp_pc;
}
//-------------------------------------------------------------
uint8_t at_get_resp_timeout_flag(void)
{
  return 0;//at_wait_resp_timeout;
}


void at_timeout_start(void)
{
  at_timeout_status = ENABLED;
  return;
}
void at_timeout_stop(void)
{
  at_timeout_status = DISABLED;
  return;
}
uint8_t at_get_timeout_status(void)
{
  return at_timeout_status;
}



void at_clear_atd_stat_reg(void)
{
  atd_resp_stat_reg = 0;
  return;
}

uint8_t at_is_busy(void)
{
  uint8_t result = 0;
  if (bit_is_set(atd_resp_stat_reg, AT_BUSY)) {
    result = 1;
  }
  return result;
}

uint8_t at_is_no_answer(void)
{
  uint8_t result = 0;
  if (bit_is_set(atd_resp_stat_reg, AT_NO_ANSWER)) {
    result = 1;
  }
  return result;
}

uint8_t at_is_no_carrier(void)
{
  uint8_t result = 0;
  if (bit_is_set(atd_resp_stat_reg, AT_NO_CARRIER)) {
    result = 1;
  }
  return result;
}

uint8_t at_is_no_dialtone(void)
{
  uint8_t result = 0;
  if (bit_is_set(atd_resp_stat_reg, AT_NO_DIALTONE)) {
    result = 1;
  }
  return result;
}
//-------------------------------------------------------------
double at_get_balance(char *balanceno)
{
	if (NULL == balanceno) 
		return 0.0;

	char cmd[16] = {0};
    sprintf(cmd, "ATD\#%s\#;", balanceno); 	
	at_send_req(cmd);
    _delay_ms(300);
	char token[16] = {0};
	char pos;
  	char scann_state = 0;
	char done;
  	char resp_rcv = 0;
  	char i = 0;
	char c; 
	char balance_fetched = 0;
	double balance = 0.0; 
	while (!resp_rcv && !balance_fetched) {
		done = 0;
		pos = 0;		
		while (!done) {
			// �������� ��������� ������ �� ������ �����
			// �������� ��������������� ��������
			//c = resp[i++];
			green_led_off();
	  		do {
	    		green_led_on();
				c = uart0_getc();		
      		} while (!c/* && !at_timeout*/); // ����� �������
	  		green_led_off();

      		switch (scann_state) {
			case 0:
				if ('\r' == c) continue;
				else if ('\n' == c) scann_state = 1; continue;
				break;
			case 1:
				if ('\r' == c) continue;
				else if ('\n' == c || ' ' == c) {
					if ('\n' == c) scann_state = 0;
					token[pos] = '\0';
					//__DEBUG					
					//printf("tok: [%s]\n", token);

					// ������� ���������� ������ ��������� ������ �� ���������� �������:
     				// ������ =	[��������] ����� [����������].
					// �������� = "������" | "��������" | "Balance"  | "Ostatok" � �� .
					// ����� = [-]���������������.
					// ���������� = ("���" | '�' | "rub" � ��)['.'] .										
					// ��������� ������� ����� �� �������� �� �� ������ �������
					if (!balance_fetched) {
						if (isdigit(token[0]) || ('-' == token[0] && isdigit(token[1]))) {
							
							uart_puts("::$:");
							uart_puts(token);
							uart_puts("\r\n");
							
							balance = atof(token);			
							balance_fetched = 1;						
						}
					}
					done = 1;
					if (NULL != strstr(token, "OK")) resp_rcv = 1;
					//else if (NULL != strstr(token, "ERROR")) resp_rcv = 1;
					//else if (NULL != strstr(token, "+CME")) resp_rcv = 1;
		  			//else if (NULL != strstr(token, "+CMS")) resp_rcv = 1;
			    }
				break;
			}
			// �������� �����
			token[pos++] = c;
		}
  }
	return balance;
}


// ================= AT END =====================
