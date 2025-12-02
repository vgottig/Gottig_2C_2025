/*! @mainpage Práctica
 *
 * @section genDesc General Description
 *
 * Se pretende diseñar un dispositivo que se utilizará para medir la temperatura de individuos en la entrada de la FI-UNER.
 * Dicho dispositivo cuenta con una termopila para el sensado de temperatura, con su correspondiente circuito de 
 * acondicionamiento. Cuenta además con un sensor de ultrasonido HC-SR04 para medir la distancia de la persona a la 
 * termopila. Estos sensores trabajan midiendo la temperatura de un objeto a la distancia, siendo esta distancia 
 * preestablecida (según el sensor, lentes, etc.) para obtener datos de temperatura correctos. Para la termopila utilizada
 * esa distancia es de 10cm ±2cm.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral      |   ESP32   	          |
 * |:------------------:|:------------------------|
 * | 	HcSr04 	        | 	GPIO_20 y GPIO_21	  |
 * | SENSOR_TEMPERATURA |       	CH1		      |
 * |    GPIO_ALARMA     |       	GPIO_9		  |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Valentina Gottig (valentinagottig@gmail.com)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "hc_sr04.h"
#include "led.h" 
#include "analog_io_mcu.h"
#include "uart_mcu.h"


#define TIME_PERIOD 1000000 // 1 segundo
#define TIME_PERIOD2 100000 // 100 ms

#define SENSOR_TEMPERATURA CH1
#define GPIO_ALARMA GPIO_9


#define VOLTAJE_MIN 0.0
#define VOLTAJE_MAX 3300
#define TEMPERATURA_MIN 20.0
#define TEMPERATURA_MAX 50.0
#define TEMP_OBJEIVO 37.5
/*==================[macros and definitions]=================================*/
uint16_t distancia;
float promedio;
float suma;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
TaskHandle_t task_handle = NULL;
TaskHandle_t task_handle2 = NULL;

/** @fn  void Notify(void *param)
 * @brief  notifica a la tareas task_handle
 * @param *param
 */
void Notify(void *param)
{
	vTaskNotifyGiveFromISR(task_handle, pdFALSE);
}

/** @fn  void Notify(void *param)
 * @brief  notifica a la tareas task_handle1
 * @param *param
 */
void Notify2(void *param)
{
	vTaskNotifyGiveFromISR(task_handle2, pdFALSE);
}

/** @fn  ConvertirVoltajeATemperatura(uint16_t voltaje)
 * @brief  convierte los valores de voltaje a temperatura
 * @param voltaje
 */
uint16_t ConvertirVoltajeATemperatura(uint16_t voltaje)
{
	return TEMPERATURA_MIN + (voltaje - VOLTAJE_MIN) * (TEMPERATURA_MAX - VOLTAJE_MIN) / (VOLTAJE_MAX - VOLTAJE_MIN);
}

/** @fn  void activarAlarma()
 * @brief  activa la alarma
 */
void activarAlarma()
{
	GPIOOn(GPIO_ALARMA);
}

/** @fn  void desactivarAlarma()
 * @brief  activa la alarma
 */
void desactivarAlarma()
{
	GPIOOff(GPIO_ALARMA);
}

/** @fn  void medirDistancia(void *pvParameter)
 * @brief  Tarea que mide la distancia y prende el led necesario dependiendo de la distancia
 * @param .void *pvParameter
 */
void medirDistancia(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // La tarea espera en este punto hasta recibir una notificación

		distancia = HcSr04ReadDistanceInCentimeters(); 

		if (distancia < 8)
		{
			LedOn(LED_1);
		}
		else if (distancia >=8 && distancia <=12)
		{
			LedOn(LED_2);
		}	
		else if (distancia > 12 && distancia < 140)
		{
	        LedOn(LED_3);
		}
		else if (distancia > 140) // se reiniciar el ciclo de medidas
		{
			suma=0.0;
		}
	}
}	
	
/** @fn  void medirTemperatura(void *pvParameter)
 * @brief  Tarea que mide la temperatura cuando se encuentra en la distancia correcta. 
 *         Ademas, calcula el promedio de las 10 mediciones y las informa por UART junto con su distancia
 * @param .void *pvParameter
 */
void medirTemperatura(void *pvParameter)
{
	uint16_t temperatura_mV;
	uint16_t temperatura;

	int numMediciones = 10;
    float mediciones[numMediciones];
	float suma = 0.0;

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (distancia >=8 && distancia <=12)
	{
		AnalogInputReadSingle(SENSOR_TEMPERATURA, &temperatura_mV); 
		temperatura = ConvertirVoltajeATemperatura(temperatura_mV);

		 for(int i = 0; i < numMediciones; i++)
		{
        mediciones[i] = temperatura;
		suma += mediciones[i];
		float promedio = suma / numMediciones;
	    }	

	    // MESAJES POR LA UART
		UartSendString(UART_PC, (char *) UartItoa(promedio, 10));
        UartSendString(UART_PC, "Cº");
        UartSendString(UART_PC, (char *) UartItoa(distancia, 10));
        UartSendString(UART_PC, "cm\r\n");

		if (temperatura < TEMP_OBJEIVO )
		{
			desactivarAlarma();
		}
		else if (temperatura > TEMP_OBJEIVO)
		{
			activarAlarma();
		}
	}
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){

	// configuro timer 1 para el control de las distancias
	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = TIME_PERIOD,
		.func_p = Notify,
		.param_p = NULL};

	TimerInit(&timer_1); // inicializo timer1

		timer_config_t timer_2 = {
		// configuro timer 2 para el control de la temperatura
		.timer = TIMER_B,
		.period = TIME_PERIOD2,
		.func_p = Notify2,
		.param_p = NULL};

	TimerInit(&timer_2); // inicializo timer 2

	TimerStart(timer_1.timer); // para que comience el timer 1
	TimerStart(timer_2.timer); // para que comience el timer 2

	HcSr04Init(GPIO_20, GPIO_21); 
	GPIOInit(GPIO_ALARMA, GPIO_INPUT);

	LedsInit();

xTaskCreate(&medirDistancia, "medir distancias", 2048, NULL, 5, &task_handle); 
xTaskCreate(&medirTemperatura, "medir temperatura", 2048, NULL, 5, &task_handle2); 
}
/*==================[end of file]============================================*/