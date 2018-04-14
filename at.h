/* * * * * *  
 *  at.h - header for at interface lib
 *
 *  AT - AT interface Library
 *
 *  Copyright (C) 2014  Michael DARIN/ћихаил ƒарьни
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
 *  Michel Darin may be contacted by email at:
 *     m-dot-darin-at-email-dot-su
 *
 * * */
#ifndef AT_H
#define AT_H

//  ============== AT interface =============
//  
//  верси€ 0.41
//  дата 30.04.2014

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "include/uart/uart.h"
#include "include/uart/uart_addon.h"
// __DEBUG ”далить!
#include "include/led.h"
#include <util/delay.h>

#define AT_RESP_OK 0
#define AT_RESP_ERROR 1
// подумать на ошибка с кодами


#define AT_WAIT_PRESCALER_MAX 300 // подобрабть 
#define AT_RESP_STACK_DEPTH 10

void at_init_resp(void);
// функци€ отключена за ненадобностью
//void at_destroy_resp_params(void);
void at_destroy_resp(void);
void at_send_req(char *at_cmd);
uint8_t at_get_resp(void); 
uint8_t at_parse_resp(void);


// вариант 1
char **at_get_resp_params(void);


void at_rise_resp_timeout_flag(void);
void at_clear_resp_timeout_flag(void);

uint8_t at_get_resp_count(void);
uint8_t at_get_resp_pc(void);
uint8_t at_get_resp_timeout_flag(void);

void at_timeout_start(void);
void at_timeout_stop(void);
uint8_t at_get_timeout_status(void);

void at_clear_atd_stat_reg(void);
uint8_t at_is_busy(void);
uint8_t at_is_no_answer(void);
uint8_t at_is_no_carrier(void);
uint8_t at_is_no_dialtone(void);
//new!
double at_get_balance(char *balnceno);

// ============= AT glabal =========
//
//
//
//static uint8_t at_wait_prescaler;
//static 
//volatile 
//uint8_t at_wait_resp_timeout = 1;

volatile uint8_t at_prescaler;
volatile uint8_t at_timeout;
// массивы можно сделать динамическими **at_response и **at_resp_params 
// или более продвинутыми типами данных: списки, стеки...
static uint8_t at_resp_count = 0; // количество записей в списке ответа 
static char *at_response[AT_RESP_STACK_DEPTH] = {0}; // весь ответ
static char *at_resp_params[AT_RESP_STACK_DEPTH] = {0}; // возможные параметры ответа
static uint8_t at_resp_pc = 0;

// регистр статуса ответа комадны ATD
#define AT_BUSY 0
#define AT_NO_ANSWER 1
#define AT_NO_CARRIER 2
#define AT_NO_DIALTONE 3
// 4:7 быты заререзвированы
static uint8_t atd_resp_stat_reg = 0;
char at_sms;


#define DISABLED 0
#define ENABLED 1
static uint8_t at_timeout_status = DISABLED;



#endif // AT_H
