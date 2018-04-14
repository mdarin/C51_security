#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif

// atmega32


//#if defined(__AVR_ATmega168__ || __AVR_ATmega328__)
#define GREEN_LED_PIN    PD6
#define RED_LED_PIN      PD7       
//#endif



#define LED_DDR DDRD
#define LED_PORT PORTD

#define leds_init() LED_DDR |= (1 << GREEN_LED_PIN) | (1 << RED_LED_PIN)

#define green_led_on() {LED_PORT |= (1<<GREEN_LED_PIN);} // Включить зеленый диод.
#define green_led_off() {LED_PORT &= ~(1<<GREEN_LED_PIN);} // Выключить зеленый диод.

#define red_led_on() {LED_PORT |= (1<<RED_LED_PIN);} // Включить красный диод.
#define red_led_off() {LED_PORT &= ~(1<<RED_LED_PIN);} // Выключить красный диод.

#ifdef __cplusplus
}
#endif

#endif// LED_H_
