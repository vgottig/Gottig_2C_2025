/*! @mainpage Guía 2. Ejercicio 1
 *
 * @section genDesc General Description
 *
 * Diseñar el firmware modelando con un diagrama de flujo
 * de manera que cumpla con las siguientes funcionalidades:
 * 
 * 1) Mostrar distancia medida utilizando los leds de la siguiente manera:
 * 
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 
 * 2) Mostrar el valor de distancia en cm utilizando el display LCD.
 * 3) Usar TEC1 para activar y detener la medición.
 * 4) Usar TEC2 para mantener el resultado (“HOLD”).
 * 5) Refresco de medición: 1 s.
 * 
 * Se deberá conectar a la EDU-ESP un sensor de ultrasonido HC-SR04
 * y una pantalla LCD y utilizando los drivers provistos por la cátedra 
 * implementar la aplicación correspondiente.
 * Se debe subir al repositorio el código. Se debe incluir en la documentación,
 * realizada con doxygen, el diagrama de flujo. 
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * |    TRIGGER     |   GPIO_2      |
 * |    +5V         |   +5V         |
 * |    GND         |   GND         |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/09/2025 | Document creation		                         |
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
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "hc_sr04.h"
#include "delay_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
#define CONFIG_BLINK_PERIOD_2 50

/*==================[internal data definition]===============================*/

/** @def tecla1
*   @brief Activa y desactiva la medición
*/ 
bool tecla1 = false; 

/** @def tecla2
*   @brief Mantiene el resultado de la medición
*/ 
bool tecla2 = false;

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

/** @fn void distanciasTask(void *pvParameter)
 *  @brief Tarea que mide la distancia utilizando el sensor y los leds
 *  @param[in] void *pvParameter
 *  @return 
*/
void distanciasTask(void *pvParameter)
{
	uint8_t distancia;

	while (true)
	{
		if (tecla1) // Si la medición está activada, lee la distancia
		{
			distancia = HcSr04ReadDistanceInCentimeters(); // Para que me de la distancia en cm

			if (!tecla2)
			{								 // Si la tecla 2 no está pulsada
				LcdItsE0803Write(distancia); // Muestra la distancia en el LCD
			}

			if (distancia < 10) // Si la distancia es menor a 10 cm se apagan todos los leds
			{
				LedsOffAll();
			}

			else if (distancia > 10 && distancia < 20) // Si la distancia está entre
			{										  // 10 y 20, se prende el led 1 y
				LedOn(LED_1);                         // se apagan los led 2 y led 3
				LedOff(LED_2);
				LedOff(LED_3);
			}

			else if (distancia > 20 && distancia < 30) // Si la distancia está entre 20 y 30
			{							               // se prenden los led 1 y 2
				LedOn(LED_1);			               // y se apaga el led 3
				LedOn(LED_2);
				LedOff(LED_3);
			}

			else if (distancia > 30) // Si la distancia es mayor a 30
			{					     // se prenden los tres leds
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
		}
		else
		{
			LcdItsE0803Off(); // Apaga el LCD
			LedsOffAll(); // Apaga los tres leds
		}

		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); // Cada 1000 ms
	}
}

/** @fn void controlTeclasTask(void *pvParameter)
 *  @brief Tarea que activa y detiene la medición utilizando la tecla 1 y mantiene un resultado utilizando la tecla 2
 *  @param[in] void *pvParameter
 *  @return 
*/
void controlTeclasTask(void *pvParameter)
{
	uint8_t teclas;

	while (true)
	{
		teclas = SwitchesRead(); // Le asigno el estado de las teclas (switches)

		switch (teclas) // Reviso el estado de las teclas
		{
		case SWITCH_1:
			tecla1 = !tecla1; // Si se pulsa la tecla 1, ahora vale true. Mide y muestra
			break;
		case SWITCH_2:
			tecla2 = !tecla2; // Si se pulsa la tecla 2, ahora vale true
			                 // LCD mantiene fijo el valor aunque el sensor siga midiendo
			break;
		}

		vTaskDelay(CONFIG_BLINK_PERIOD_2 / portTICK_PERIOD_MS); // Cada 50 ms
	}
}

void app_main(void){
	// Inicializo todos los periféricos: LEDs, LCD, sensor y teclas
	LedsInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();

	// Creo las tareas
	xTaskCreate(&distanciasTask, "distanciasTask", 512, NULL, 5, NULL);
	xTaskCreate(&controlTeclasTask, "controlTeclasTask", 512, NULL, 5, NULL);	
}
/*==================[end of file]============================================*/