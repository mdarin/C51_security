#ifndef HANDSFREE_H
#define HANDSFREE_H
#define handsfree_uart_disable()  UCSR0A = 0; UCSR0B = 0; UCSR0C = 0;  
#define handsfree_enable() PORTD |= (1 << PD2);
#define handsfree_init()  DDRD |= (1 << PD2);
#define handsfree_disable()  PORTD &= ~(1 << PD2);
#endif //HANDSFREE_H
