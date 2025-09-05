/*! @mainpage Guia 1. Ejercicio 6
 *
 * @section genDesc General Description
 *
 * Escriba una función que reciba un dato de 32 bits,
 * la cantidad de dígitos de salida y dos vectores de estructuras
 * del tipo  gpioConf_t. Uno  de estos vectores es igual al definido
 * en el punto anterior y el otro vector mapea los puertos
 * con el dígito del LCD a donde mostrar un dato:
 * 
 * Dígito 1 -> GPIO_19
 * Dígito 2 -> GPIO_18
 * Dígito 3 -> GPIO_9
 * 
 * La función deberá mostrar por display el valor que recibe.
 * Reutilice las funciones creadas en el punto 4 y 5.
 * Realice la documentación de este ejercicio usando Doxygen.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    EDU-ESP     |   Peripheral 	|
 * |:--------------:|:--------------|
 * | 	GPIO_20	 	| 	  D1	    |
 * | 	GPIO_21	 	|  	  D2 	   	|
 * | 	GPIO_22	 	|  	  D3	   	|
 * | 	GPIO_23	 	|  	  D4 	   	|
 * | 	GPIO_19	 	|  	  SEL_1	   	|
 * | 	GPIO_18	 	|  	  SEL_2   	|
 * | 	GPIO_9  	|  	  SEL_3   	|
 * | 	+5V	 	    |  	  +5V   	|
 * | 	GND 	 	|  	  GND   	|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/09/2025 | Document creation		                         |
 *
 * @author Valentina Gottig (valentina gottig@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/**
 * @brief Estructura que mapea un GPIO y su dirección
 */
typedef struct {
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

/** @fn int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number);
 *  @brief Convierte el dato recibido a BCD y guarda cada uno de los dígitos 
 * 		   de salida en el arreglo pasado como puntero.
 *  @param[in] uint32_t data, uint8_t digits, uint8_t * bcd_number
 *  @return int8_t 0 
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t * bcd_number) {
    // Inicializar el array de salida con ceros
    // Esto es útil si el número de entrada tiene menos dígitos que 'digits'.
    for (int i = 0; i < digits; ++i) {
        bcd_number[i] = 0;
    }

    // Usar un índice para rellenar el array de derecha a izquierda
    int8_t index = digits - 1;

    // Bucle para extraer cada dígito del número
    while (data > 0 && index >= 0) {
        // Obtener el último dígito (el residuo de la división por 10)
        uint8_t digit = data % 10;
        
        // Guardar el dígito en el array
        bcd_number[index] = digit;
        
        // Moverse al siguiente dígito
        data /= 10;
        index--;
    }
    
    return 0; // Devuelve 0 para indicar que la operación fue exitosa
}

/** @fn void functionBCD(uint8_t digit, gpioConf_t *vector);
 *  @brief Cambia el estado de cada GPIO, a ‘0’ o a ‘1’, según el estado del bit
 *         correspondiente en el BCD ingresado.
 *  @param[in] uint8_t digit, gpioConf_t *vector
 *  @return 
*/
void functionBCD(uint8_t digit, gpioConf_t *vector){ // Recibe un dígito (valor numérico)
													 // y un vector que contiene la configuración de b0 a b3
	for(uint8_t i=0; i<4; i++){
		GPIOInit(vector[i].pin,vector[i].dir); // Inicializa cada pin en la dirección indicada del vector
	}

	// Evalúa cada bit
	for(int i=0; i<4; i++){
		if((digit&(1<<i))==0){ // Comprueba si el bit i de digit está en 0 o en 1. << (shifteo/corrimiento a la derecha)
			GPIOOff(vector[i].pin); // Si está en 0 llama a GPIOOff y escribe nivel bajo
		}
		else
			GPIOOn(vector[i].pin); // Si está en 1 llama a GPIOOn y escribe nivel alto
	}
}

/** @fn void showDisplay(uint32_t dato, uint8_t digitos, gpioConf_t *vectorPines, gpioConf_t *vectorPuertos);
 *  @brief Muestra por display el valor que recibe.
 *  @param[in] uint32_t dato, uint8_t digitos, gpioConf_t *vectorPines, gpioConf_t *vectorPuertos
 *  @return 
*/
void showDisplay(uint32_t dato, uint8_t digitos, gpioConf_t *vectorPines, gpioConf_t *vectorPuertos){

	uint8_t array[3];

	convertToBcdArray(dato,digitos,array);

	for(int i=0; i<digitos; i++){
		functionBCD(array[i],vectorPines);
		GPIOOn(vectorPuertos[i].pin);
		GPIOOff(vectorPuertos[i].pin);
	}
}

void app_main(void){
	/** Arreglo que mapea los GPIO de los bits.
*/
	gpioConf_t vectorPines[4]={
		{GPIO_20,GPIO_OUTPUT},
		{GPIO_21,GPIO_OUTPUT},
		{GPIO_22,GPIO_OUTPUT},
		{GPIO_23,GPIO_OUTPUT}
	};

	for(uint8_t i=0; i<4; i++){
		GPIOInit(vectorPines[i].pin,vectorPines[i].dir);
	}

/** Declara e inicializa un arreglo de pines de 3 elementos de tipo gpioConf_t.
 * Mapeo de los GPIOs
*/
gpioConf_t vectorPuertos[3]={
		{GPIO_19,GPIO_OUTPUT},
		{GPIO_18,GPIO_OUTPUT},
		{GPIO_9,GPIO_OUTPUT},
};

	 for(uint8_t i=0; i<3; i++){
		GPIOInit(vectorPuertos[i].pin,vectorPuertos[i].dir); // Inicializa cada pin en la dirección indicada del vector
	}

	uint32_t num = 257;
	uint8_t digitos = 3;

	showDisplay(num, digitos, vectorPines, vectorPuertos);
}
/*==================[end of file]============================================*/