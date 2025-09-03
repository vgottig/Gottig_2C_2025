/*! @mainpage Guia 1. Ejercicio 5
 *
 * @section genDesc General Description
 *
 * Escribir una función que reciba como parámetro un dígito BCD
 * y un vector de estructuras del tipo gpioConf_t.
 * Incluya el archivo de cabecera gpio_mcu.h
 * 
 * Defina un vector que mapee los bits de la siguiente manera:
 * b0 -> GPIO_20
 * b1 -> GPIO_21
 * b2 -> GPIO_22
 * b3 -> GPIO_23
 * 
 * La función deberá cambiar el estado de cada GPIO, a ‘0’ o a ‘1’,
 * según el estado del bit correspondiente en el BCD ingresado.
 * Ejemplo: b0 se encuentra en ‘1’, el estado de GPIO_20 debe setearse. 
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
 * | 03/09/2025 | Document creation		                         |
 *
 * @author Valentina Gottig (valentinagottig@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct {
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void functionBCD(uint8_t digit, gpioConf_t *vector){ // Recibe un dígito (valor numérico)
													 // y un vector que contiene la configuración de b0 a b3
	for(uint8_t i=0; i<4; i++){
		GPIOInit(vector[i].pin,vector[i].dir); // Inicializa cada pin en la dirección indicada del vector
	}

	// Evalúa cada bit
	for(int i=0; i<4; i++){
		if((digit&(1<<i))==0){ // Comprueba si el bit i de digit está en 0 o en 1
			GPIOOff(vector[i].pin); // Si está en 0 llama a GPIOOff y escribe nivel bajo
		}
		else
			GPIOOn(vector[i].pin); // Si está en 1 llama a GPIOOn y escribe nivel alto
	}
}

void app_main(void){
	// Declara e inicializa un arreglo de pines de 4 elementos de tipo gpioConf_t
	gpioConf_t vectorPines[4]={
		{GPIO_20,GPIO_OUTPUT},
		{GPIO_21,GPIO_OUTPUT},
		{GPIO_22,GPIO_OUTPUT},
		{GPIO_23,GPIO_OUTPUT}
	};
	functionBCD(9,vectorPines); // Llama a functionBCD con el digito dado y el vector de pines
								// Muestra un 9
}
/*==================[end of file]============================================*/