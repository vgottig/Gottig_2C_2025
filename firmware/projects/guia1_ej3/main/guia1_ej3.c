/*! @mainpage Guia 1. Ejercicio 3
 *
 * @section genDesc General Description
 *
 * Realice un función que reciba un puntero a una estructura LED
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Valentina Gottig (valentinagottig@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
#define TOGGLE 2
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
struct leds
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de encendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds;

void ledControl(struct leds *my_leds) {
    for (uint8_t i = 0; i < my_leds->n_ciclos; i++) {
        switch (my_leds->mode) {
            case ON:
                LedOn(my_leds->n_led);     
                break;

            case OFF:
                LedOff(my_leds->n_led);     
                break;

            case TOGGLE:
                LedToggle(my_leds->n_led);  
                break;

            default:
                break;
        }
        vTaskDelay(my_leds->periodo / portTICK_PERIOD_MS); // Retardo entre prendido y apagado
    }
}

void app_main(void){
	LedsInit();   // Inicializamos los LEDs

    struct leds my_leds;

    my_leds.mode = TOGGLE;
    my_leds.n_led = LED_1;     // LED1=4, LED2=2, LED3=1
    my_leds.n_ciclos = 10; // Ciclo entre prendido y apagado
    my_leds.periodo = 500;     

    ledControl(&my_leds);
}
/*==================[end of file]============================================*/