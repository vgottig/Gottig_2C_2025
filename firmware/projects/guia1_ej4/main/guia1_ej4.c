/*! @mainpage Guia 1. Ejercicio 4
 *
 * @section genDesc General Description
 *
 * Escriba una función que reciba un dato de 32 bits, 
 * la cantidad de dígitos de salida y un puntero a un arreglo
 * donde se almacene los n dígitos. La función deberá convertir
 * el dato recibido a BCD, guardando cada uno de los dígitos de salida
 * en el arreglo pasado como puntero.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
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

void app_main(void){
	uint32_t number_to_convert = 12345;
    uint8_t num_digits = 8;
    uint8_t bcd_array[num_digits]; // Vector para retornar varios resultados

    printf("Convirtiendo el numero: %lu\n", number_to_convert);

    // Llama a la función
    convertToBcdArray(number_to_convert, num_digits, bcd_array);
    
    // Imprime el resultado
    printf("Resultado en BCD (almacenado en un array de %d digitos): ", num_digits);

    for (int i = 0; i < num_digits; ++i) {
        printf("%u", bcd_array[i]); 
    }
    printf("\n");
}
/*==================[end of file]============================================*/