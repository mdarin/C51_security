#include "proto.h"




// ============================================================================= 
// маркером __act обозначаются действия выволняемые командой
//
//------------------------------------------------------------------------------
char on_get_devinfo__act(packet_data_t* packet_data, packet_data_t* result)
{
	char status = 1;

	if (NULL != result) {	  
	  status = 0;
	  strcpy(result, ";dev info;FW;");
	  strcat(result, FIRMWARE_VERSION);
	  strcat(result, ";HW;");
	  strcat(result, HARDWARE_VERSION);
	  strcat(result, ";SN;");
	  strcat(result, SERIAL_NUMBER);
	  strcat(result, ";ID;");
	  strcat(result, DEVICE_ID);
	  strcat(result, ";");
	}
    
	return status;
}
//------------------------------------------------------------------------------
char on_get_sms__act(packet_data_t* packet_data, packet_data_t* result)
{
	// порядок работы
	// считать сообщение из ПЗУ
	// проверить не пустое ли, если первые пять(5) байт не равны 0xFF
	// заполнить пакет для ответа непустым сообщением
	// иначе вернуть no sms
	// иначе вернуть ошибку чтения памяти RD MEM ERR [?]
	char status = 0;
	if (NULL != result) {
	    char msg[SMS_TEXTLEN_MAX] = {0}; //0xFF debug
		strcpy(result, ";sms;");
		red_led_on();
        eeprom_busy_wait();
		eeprom_read_block(msg, &nv_msg_text, /*strlen(&nv_msg_text)+1*/ SMS_TEXTLEN_MAX);
		
		// __debug
		//uart_puts("\r\n%sms:");
		//uint16_t count = 0;
		//for (uint8_t i = 0; i < SMS_TEXTLEN_MAX; i++) {
		//  eeprom_busy_wait();
		//  msg[i] = eeprom_read_byte(&(nv_msg_text[i]));
		//  uart_puthex_byte(msg[i]);
		//}
		//uart_puts("%\r\n");
  		
        strcat(result, msg);
		strcat(result, ";");

		red_led_off();
	}
	return status;
}
//------------------------------------------------------------------------------
char on_get_phonebook__act(packet_data_t* packet_data, packet_data_t* result)
{
	//FIXME: Допилить пустые номера, если не заданы пропускать

	// порядок работы
	// считать список адресатов из ПЗУ
	// проверить не пустая ли запись, если первые пять(5) байт не равны 0xFF
	// заполнить пакет для ответа непустой записью
	// иначе вернуть no phone
	// иначе вернуть ошибку чтения памяти RD MEM ERR
	char status = 0;
	if (NULL != result) {
		strcpy(result, ";phone book;1;");
		//strcat(result, "1");	
		//strcat(result, ";");	
		char ph_no[PHONELEN_MAX] = {0};

		red_led_on();
		eeprom_busy_wait();
		eeprom_read_block(ph_no, &nv_ph_book[0], /*strlen(nv_ph_book[0])-1*/PHONELEN_MAX);
		// проверять на пустое значение
		strcat(result, ph_no);

		char buffer[4] = {0};	
		
		for (char i = 1; i < PHONEBOOKLEN_MAX; i++) {
			snprintf(buffer, 8, "%d", i+1);	
			strcat(result, ";");		
			strcat(result, buffer);		
			strcat(result, ";");		
			eeprom_busy_wait();
		    eeprom_read_block(ph_no, &nv_ph_book[i], /*strlen(nv_ph_book[0])-1*/PHONELEN_MAX);
			// проверять на пустое значение
		    strcat(result, ph_no);

		}
		red_led_off();
	}
	return status;
}
//------------------------------------------------------------------------------
char on_get_delay__act(packet_data_t* packet_data, packet_data_t* result)
{
	char status = 0;
	if (NULL != result) {
	  red_led_on();
	  uint8_t arm_delay = 0;
	  strcpy(result, ";delay;");
      eeprom_busy_wait();
	  arm_delay = eeprom_read_byte(&nv_sec_delay);
	  char buf[4] = {0};
      snprintf(buf, 8, "%d", arm_delay);
	  strcat(result, buf);
	  strcat(result, ";");
	  red_led_off();
	}
	return status;
}
//------------------------------------------------------------------------------
char on_get_devstat__act(packet_data_t* packet_data, packet_data_t* result)
{
	char status = 0;
	if (NULL != result) {
	  eeprom_busy_wait();
	  uint8_t dvsreg = eeprom_read_byte(&DVSR);
	  strcpy(result, ";dev stat;");
	  char buf[4] = {0}; 
      snprintf(buf, 8, "%d", dvsreg);
	  strcat(result, buf);
	  strcat(result, ";");
	}
	return status;
}
//------------------------------------------------------------------------------
char on_set_sms__act(packet_data_t* packet_data, packet_data_t* result)
{
  char status = 0;
  if (NULL != packet_data) {	
	char* token;
    char* delims = ";";
    // здесь производится синтасический анализ данных
    // пока примитивно, но ресурсов тоже не бесконечно...:)

    // убрать символ ETX наверное... надо пофиксить!
    packet_data[strlen(packet_data)-1] = '\0';

    token = strtok(packet_data, delims);

    // пропустить команду
    token = strtok (NULL, delims);

    while (token != NULL) {  
	  uart_puts("::");
	  uart_puts(token);
	  uart_puts("\r\n");
	  // записать новое смс сообщение в ППЗУ
	   if (!strcmp(token, "empty")) {		  
		 // считать содержимое текста смс из ПЗУ  
		 char msg[SMS_TEXTLEN_MAX] = {0}; //0xFF debug                 
		 eeprom_busy_wait();
		 eeprom_read_block(msg, nv_msg_text, strlen(token)+1);

         // возможно использовать Вариант 1 из set phone book	       
         if (strcmp(msg, "empty")) {
           //__DEBUG
		   uart_puts("WR empty\r\n");
		   red_led_on();
		   eeprom_busy_wait();
           eeprom_update_block("empty", nv_msg_text, strlen("empty")+1);
           red_led_off();
		 } else { // эту ветку убрать после отлаюки!!!
           //__DEBUG
		   uart_puts("already empty\r\n");
		 }
      } else {
	    red_led_on();
	    eeprom_busy_wait();
        eeprom_update_block(token, nv_msg_text, strlen(token)+1);	  
        red_led_off();
	  }

	  token = strtok (NULL, delims);
    }
	*result = ACK;
  }
  return status;
}
//------------------------------------------------------------------------------
char on_set_phonebook__act(packet_data_t* packet_data, packet_data_t* result)
{	
	//FIXME: Допилить пустые номера, если не заданы пропускать ГОТОВО!
	char status = 0;
	if (NULL != packet_data) {
	  //process_packet_data(packet_data);
      char* token;
      char* delims = ";";
      unsigned char index = 0;
      // здесь производится синтасический анализ данных
      // пока примитивно, но ресурсов тоже не бесконечно...:)

      // убрать символ ETX наверное... надо пофиксить!
      packet_data[strlen(packet_data)-1] = '\0';

      token = strtok(packet_data, delims);

	  // пропустить команду
      token = strtok (NULL, delims);

      while (token != NULL) {  
        //__DEBUG
        //uart_puts("[");
		//uart_puts(token);
		//uart_puts("]\r\n");
        if (!strcmp(token, "empty")) {
		  
          char tmp[PHONELEN_MAX] = {0};
          // прочитать содержимое текущей записи		  
          eeprom_busy_wait();
		  eeprom_read_block((void*)tmp, (const void*)&nv_ph_book[index], PHONELEN_MAX);

		  // вариант 1
		  // если содержимое 'empty' то пропустить запись
		 /* if (!strcmp(tmp, "empty")) { // if(strcmp(...) попробовать...
		    //__DEBUG
		    uart_puts("already empty\r\n");
		    //continue;
          } else {
		  // иначе записать 'empty'
		    uart_puts("WR empty\r\n");
			red_led_on();
		    eeprom_busy_wait();
            eeprom_update_block("empty", &nv_ph_book[index], strlen("empty")+1);
            red_led_off();
		  }*/
		  
		  // вариант 2 
          if (strcmp(tmp, "empty")) {
            //__DEBUG
			uart_puts("WR empty\r\n");
			red_led_on();
		    eeprom_busy_wait();
            eeprom_update_block("empty", &nv_ph_book[index], strlen("empty")+1);
            red_led_off();
		  } else { // эту ветку убрать после отлаюки!!!
            //__DEBUG
			uart0_puts("already empty\r\n");
		  }		   
        // если данные содержат номер телефона
	    } else if ( ('+' == token[0] || isdigit(token[0])) 
	       && isdigit(token[1])  
	       && isdigit(token[2])
	       && isdigit(token[3])
	       && isdigit(token[4])
	       && isdigit(token[5])
	       && isdigit(token[6])) {
	       // полагаем, что это номре телефона
	       // записать номер по последнему считанному индексу
	       //uart_puts("phno:");
	       //uart_puts(token);
	       //uart_puts("\r\n");
		   // записть в ППЗУ контроллера
		   red_led_on();
		   eeprom_busy_wait();
           eeprom_update_block((void*)token, (const void*)&nv_ph_book[index], strlen(token)+1);
           red_led_off();
		// если данные содержать индекс в телефонной книге
	    } else if (isdigit(token[0])) {
          //uart_puts("idx[");
	      //uart_puts(token);
	      //uart_puts("][");
		  // поулучить значение индекса
	      index = atoi(token);
		  index -= 1;
		  if (0 > index) {
		    index = 0;
		  }
	      //uart_put_int(index);
	      //uart_puts("]\r\n");
 	    } 
						
		// получить следующий токен 
	    token = strtok (NULL, delims);
      }

	  *result = ACK;
	}

	return status;
}
//------------------------------------------------------------------------------
char on_set_delay__act(packet_data_t* packet_data, packet_data_t* result)
{
	char status = 0;
	if (NULL != result) {	      
	  uint8_t sec_delay = eeprom_read_byte(&nv_sec_delay);	
      char* token;
      char* delims = ";";
      // здесь производится синтасический анализ данных
      // пока примитивно, но ресурсов тоже не бесконечно...:)

      // убрать символ ETX наверное... надо пофиксить!
      packet_data[strlen(packet_data)-1] = '\0';
      token = strtok(packet_data, delims);
	  
	  // пропустить команду
      token = strtok (NULL, delims);
      
	  while (token != NULL) { 
	   				
        if (sec_delay != atoi(token)) {      
		  red_led_on();
		  //sscanf(token,"%d",&sec_delay); //попробовать
		  // преобразовать строку в число
		  sec_delay = atoi(token);
		  // записать новое значение задержки на постановку в ППЗУ
	      eeprom_busy_wait();
          eeprom_write_byte(&nv_sec_delay, sec_delay);	  
          red_led_off();
        } //else { // ветку убрать после отдадки
          //uart_puts("already ");
		  //uart_puts(token);
		  //uart_puts("\r\n");
		//}

	    token = strtok (NULL, delims);
      }  
	}
	*result = ACK;
	return status;
}
//------------------------------------------------------------------------------
/*
// закончить запись если получена команда окончания записи
    if (NULL != strstr(packet_data, "end of track") {
	
	} else {
      // иначе записать полученные данные во внешнее ПЗУ
	  unsigned long address = packet_number * DATASZ - DATASZ;
      while (SetWriteProtectedArea(NONE) == BUSY) continue; // Disable Software Protection (ensure that #WP is high) 
      while (PutCharArray(address, PAGE_SIZE-1, &buf[3]) != TRANSFER_STARTED) continue;
	} 
*/

// =============================================================================
// Реализация транспорного уровня
//
void process_packet(packet_t *packet, conf_cmd_t *cc_list) 
{
  // FIXME: над тут вообще придумать чтобы лишних переменных не было...
  packet_data_t *data = &(packet[3]);
  // 
  // FIXME:
  //   размер этого буфера влияет на работоспособность прошивки
  //   от него по-хорошему бы вобще отказаться надо...
  // это одно из критичных мест!
  packet_data_t result[80] = {0}; 
  if (SOH == packet[0]) {
	if (ETX == packet[strlen(data)+2]) {
      if (packet[1] > 0) {
	    // если пакет правильный, то
	    // определить команду методом сопоставления со списком команд
	    for (char i = 0, done = 0; cc_list[i].comand != NULL && !done; i++) {
		  // если команда определена, то выполнить предписанные дейсвия		  
		  if (NULL != data ) {  
			if (NULL != strstr(data, cc_list[i].comand)) {			  
			  //uart_puts(data);
			  cc_list[i].action(data, result);
			  // отправить ответ с резултатом
			  send_response(packet, result);	
			  done = 1;
              // __DEBUG
			  //uart_put_int(proto_packet_number);
			  //uart_puts(" ");
		    }
		  } // else NAK
        }
	  } // else NAK
	} // else NAK
  } // else NAK
  return;
}


// FIXME: окультурить функцию после отладки
void process_stream(packet_t *packet)
{
  
  if (NULL == packet) return; // NAK
  
  packet_data_t *soundtrack = &(packet[3]);
  if (SOH == packet[0]) {
	if (ETX == packet[131]) {
      if (packet[1] > 0) {
		if (NULL != soundtrack) {  
     
            red_led_on();
	        //proto_nv_addr = proto_packet_number * 111 - 111;//128 - 128;
            uint32_t addr = proto_packet_number * 128 - 128;
	        while (SetWriteProtectedArea(NONE) == BUSY) continue; // Disable Software Protection (ensure that #WP is high) 
	        while (PutCharArray(addr, soundtrack, 128-1) != TRANSFER_STARTED) continue;
	        proto_packet_number++;

	       // __DEBUG
           //uart_puts(soundtrack);
	       //uart_puts("*");
	       //uart_put_ulongint(addr);
	       //uart_puts("*");	  
           //uart_puts("[");
	       //for (uint8_t k = 0; k < 128; k++) {
	       //  uart_puthex_byte(soundtrack[k]);
	       //}
		   //for (uint8_t k = 0; k < 132; k++) {
	       // uart_puthex_byte(packet[k]);
	       //}
	       //uart_puts("]\r\n");
           //_delay_ms(5000);           

	       
           red_led_off();
 
           // __DEBUG
		   //uart_puts(data);
		   // отправить ответ
		   uart_putc(ACK);
		   			  		  
		} // else NAK 
	  } // else NAK
	} // else NAK
  } // else NAK
  return;
}

//------------------------------------------------------------------------------
char create_packet(packet_t* packet, packet_data_t* data)
{
	char status = 1;	
	if (NULL != packet && NULL != data) {
	  status = 0;
	  char pdl = strlen(data);
	  packet[0] = SOH;
	  packet[1] = 1;
	  packet[2] = pdl;
	  strcpy(&(packet[3]), data);
	  packet[4+pdl-1] = ETX; // -1 нужен чтобы отсечь символ \0 терминатор
	}
	return status;
}
//------------------------------------------------------------------------------
void send_response(packet_t* packet, packet_data_t* data) 
{
	if (NULL == data) {		
	  data = "";		
	}

	switch (data[0]) {
	case ACK:
	  uart0_putc(ACK);
	  break;
	case NAK:
	  uart0_putc(NAK);
	  break;
	default:
	  create_packet(packet, data);
	  uart0_puts(packet);
	  break;
	}	
	
	return;
}
//------------------------------------------------------------------------------
/*char wait_for_data(void)
{
  char c;
  do { 
  	c = uart0_getc();
  } while (!c);
  return c;
}*/

//------------------------------------------------------------------------------
/*void receive(void) 
{	
	char packet_pos = 0;
  	packet_t packet[132] = {0};
  	char c;
  	char done = 0;
	 
	switch (proto_state) {
	case ESTABLISH: // ----------------------
	  while (!done) {
	    //uart0_flush();
		wait_for_data(c);
	    if (ENQ == c) {
	      uart_putc(ACK);
	  	  done = 1;
		  proto_state = INTERACT;
		  green_led_on();
	    }	 
	  }
	  break;
	case INTERACT: // ----------------------
	  while (!done) {
	    wait_for_data(c);
	    switch (c) {
		case SOH:
	      packet[packet_pos++] = c;
		  // exit on success or timeout should be added!!!
		  do {
		    wait_for_data(c);
		    packet[packet_pos++] = c;			
          } while (ETX != c && packet_pos < 132);
		  process_packet(packet, cc_list);
		  packet_pos = 0;
		  done = 1;
		  break;
		case EOT:
		  packet_pos = 0;
		  proto_state = TERMINATE;
		  done = 1;
		  break;
		}  		 
	  }
	  break;
	case TERMINATE: // ----------------------
	  uart0_putc(ACK);
	  proto_state = ESTABLISH;
	  green_led_off();
	  break;
	default:
	  // ...
	  break;
	
	}	
	return;
}*/
//------------------------------------------------------------------------------
void receive(void)
{
	packet_t packet[PACKET_SIZE] = {0};
	uint8_t packet_pos = 0;
	char c;
	char done = 0;
	//do {	
		switch (proto_state) {
		case ESTABLISH: // ----------------------------------------------
		  while(!done) {	
			do {
				c = uart_getc();
			} while (!c);
			if (ENQ == c) {
				done = 1;
				green_led_on();
				proto_state = INTERACT;
				uart_putc(ACK);				
			}
          }
		  break;
		case INTERACT: // ----------------------------------------------
		  while (!done) {	
			do {
				c = uart_getc();
			} while (!c);
			switch (c) {
			case DLE:
				//__DEBUG
                uart_puts("OK DLE\r\n");

				done = 1;
				//proto_nv_addr = 0;
	            proto_packet_number = 1;
				packet_pos = 0;
	            //proto_start_rcv = 1;
				
				// очищать чип и производить подготовку к приёму файла
				red_led_on();
		        init_spi_master(); // вынесено в startup_init()
                asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");			
				EraseChip();

				uart_puts("OK Chip erased\r\n");
				red_led_off();
	
				uart_puts("Writing...->STREAM\r\n");
				proto_state = STREAM;
				uart_putc(ACK);
				break;
			case SOH:
				packet[packet_pos++] = c;
				do {
					do {
						c = uart_getc();
					} while (!c);	
					packet[packet_pos++] = c;
				} while (ETX != c && packet_pos < PACKET_SIZE);
				process_packet(packet, cc_list);
				packet_pos = 0;
				break;
			case EOT:
				done = 1;
				proto_state = TERMINATE;
				uart_putc(ACK);
				break;		
			}
		  } //eof while
		  break;
		case STREAM: // ----------------------------------------------
		  while (!done) {	
			do {
				c = uart_getc();
			} while (!c);
			switch (c) {
			case SOH:
				packet[packet_pos++] = c;
				//uart_putc(c);
				do {
					do {
						c = uart_getc();
					} while (!c);
					//uart_putc(c);	
					packet[packet_pos++] = c;					
				} while (packet_pos < PACKET_SIZE);
				process_stream(packet);
				packet_pos = 0;			  
			   break;
			case FS:			  	
				do {
					c = uart_getc();
				} while (!c);
				if (FF == c) {
					do {
						c = uart_getc();
					} while (!c);
					if (EOT == c) {
						done = 1;
						proto_state = INTERACT;
						proto_nv_addr = 0;
	                    proto_packet_number = 1;
	                    proto_start_rcv = 1;
						uart_putc(ACK);
						uart_puts("EOF");
					}
				} // else process next file... on v.3
				break;
			}
		  }	//eof while 
		  break;
		case TERMINATE: // ----------------------------------------------	
			green_led_off();
			proto_state = ESTABLISH;
			break;	
		}
	//} while (!done);

	return;
}
//------------------------------------------------------------------------------
