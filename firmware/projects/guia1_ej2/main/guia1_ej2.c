/*! @mainpage Guia 1. Ejercicio 2
 *
 * @section genDesc General Description
 *
 * Modifique la aplicación 1_blinking_switch de manera de hacer titilar
 * los leds 1 y 2 al mantener presionada las teclas 1 y 2 correspondientemente.
 * También se debe poder hacer titilar el led 3 al presionar simultáneamente
 * las teclas 1 y 2.
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
 * | 06/08/2025 | Document creation		                         |
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
#define CONFIG_BLINK_PERIOD 1000 
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas; //guardar teclas
	LedsInit(); //inicializar
	SwitchesInit();
    while(1)    {
    	teclas  = SwitchesRead(); //llama a una función de teclas (lee que tecla está apretada)
    	switch(teclas){ //entro en un switch
    		case SWITCH_1: //si es la tecla 1,
    			LedToggle(LED_1); //cambia de estado (si está apagada se prende y viceversa)
    		break; //sale de la llave, sigue ejecutando los siguientes casos

    		case SWITCH_2:
    			LedToggle(LED_2);
    		break;

			case SWITCH_1 | SWITCH_2:
				LedToggle(LED_3);
			break;
    	}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}