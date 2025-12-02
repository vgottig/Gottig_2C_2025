/*! @mainpage Práctica
 *
 * @section genDesc General Description
 *
 * El siguiente programa consiste en un sistema de control de pH y humedad. 
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |     ESP32      |   Peripheral 	|
 * |:--------------:|:--------------|
 * | 	Bomba agua 	| 	GPIO_15		|
 * | 	Bomba pHA 	| 	GPIO_16		|
 * | 	Bomba pHB 	| 	GPIO_17		|
 * | Sensor humedad | 	GPIO_18		|
 * | 	Sensor pH 	| 	CH1 		|
 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author @author Valentina Gottig (valentinagottig@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "switch.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
/*Tiempo al que quiero que se realice el sensado */
#define CONFIG_BLINK_PERIOD_SENSADO 3000000

/*Tiempo al que quiero que se muestren los mensajes por la UART */
#define CONFIG_BLINK_PERIOD_INFORME 5000000

/*Periodo al que quiero que lea lo que le mando por las teclas.*/
#define CONFIG_BLINK_PERIOD_TECLAS 200

/*Defino GPIO para bomba de agua */
#define GPIO_BOMBA_AGUA GPIO_15

/*Defino GPIO para bomba de pHA */
#define GPIO_BOMBA_PHA GPIO_16

/*Defino GPIO para bomba de pHB */
#define GPIO_BOMBA_PHB GPIO_17

/*Defino GPIO para Sensor de humedad */
#define GPIO_SENSOR_RIEGO GPIO_18

/* handler de la tarea para sensar el pH */
TaskHandle_t sensarphtask_task_handle = NULL;
/* handler de la tarea para sensar la humedad */
TaskHandle_t sensarhumedadtask_task_handle = NULL;
/* handler de la tarea para mostrar los mensajes */
TaskHandle_t mostrarmensajetask_task_handle = NULL;

/*==================[internal data definition]===============================*/
/*Me guarda el valor de  voltaje que me da el sensor de pH*/
uint16_t valor_sensado;

/*Me guarda el valor de pH que obtengo realizando una regla de 3 simples con el voltaje que mide el sensor*/
uint16_t data;

/*Si el GPIO del sensor de humedad esta en alto es TRUE, sino es FALSE */
bool humedad; 

/*Si es TRUE inicia el sistema, si es FALSE lo apaga.*/
bool encendido;

/*guarda el valor de la tecla que se presiona */
uint8_t teclas;
/*==================[internal functions declaration]=========================*/
/**
 * @fn static void SensarPHTask(void *pvParameter)
 * @brief Lee el valor de voltaje que me da el sensor de pH, lo convierte y lo guarda en una variable
 * @param[in] void *pvParameter
 */
static void SensarPHTask(void *pvParameter);

/**
 *	@fn static void SensarHumedadTask(void *pvParameter)
 *  @brief Activa o desactiva la bomba de agua dependiendo lo que me da el sensor de humedad  
 *  @param[in] void *pvParameter
 * 
 */
static void SensarHumedadTask(void *pvParameter);

/**
 * @fn void PrenderBombas()
 * @brief Prende las bombas de pH dependiendo el valor de pH que mide el sensor
 * 
 */
void PrenderBombas();

/**
 * @fn void MostrarMensajeTask(uint8_t mensaje)
 * @brief Tarea para mostrar los mensajes por la uart
 * @param uint8_t mensaje le paso un numero que le va a indicar que mensaje poner
 * 
 */
void MostrarMensajeTask(uint8_t mensaje);

/**
 * @fn void FuncTimer(void *pvParameter);
 * @brief Notifica a las tareas que refieren al sensado cuando deben ser interrumpidas.
 * @param[in] void *pvParameter
 * @return 
*/
void FuncTimer(void* param);

/**
 * @fn void FuncTimer(void *pvParameter);
 * @brief notifica a la tarea para mostrar mensajes por uart cuando debe ser interrumpida.
 * @param[in] void *pvParameter
 * @return 
*/
void FuncTimerOut(void *param);

/**
 * @fn void LeerTeclas();
 * @brief Determina las acciones a realizar si se apreta una tecla u otra.
 * @param[in]
 * @return
 */
void LeerTeclas();

/*==================[external functions definition]==========================*/
static void SensarPHTask(void *pvParameter)
{
	if (encendido == true)
	{
		AnalogInputReadSingle(CH1, &valor_sensado);
		data = valor_sensado*14/3; // Convierto Voltaje a valor de pH
	}
}

static void SensarHumedadTask(void *pvParameter)
{
	if(encendido ==true)
	{
		humedad = GPIORead(GPIO_SENSOR_RIEGO);
		if(humedad)
		{
			GPIOOff(GPIO_BOMBA_AGUA);
			MostrarMensajeTask(2);
		}
		else 
		{
			GPIOOn(GPIO_BOMBA_AGUA);
			MostrarMensajeTask(1);
			MostrarMensajeTask(5);
		} 
	}	
}

void PrenderBombas()
{
	if(data<6)
	{
		GPIOOn(GPIO_BOMBA_PHB);
		GPIOOff(GPIO_BOMBA_PHA);
		MostrarMensajeTask(4);
	}

	else if(data > 6.7)
	{
		GPIOOn(GPIO_BOMBA_PHA);
		GPIOOff(GPIO_BOMBA_PHB);
		MostrarMensajeTask(3);
	}

	else MostrarMensajeTask(6);
}

void FuncTimer(void* param)
{
    vTaskNotifyGiveFromISR(sensarphtask_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(sensarhumedadtask_task_handle, pdFALSE);
}

void FuncTimerOut(void *param)
{
	vTaskNotifyGiveFromISR(mostrarmensajetask_task_handle, pdFALSE);
}

void MostrarMensajeTask(uint8_t mensaje)
{ 
	if(encendido)
	{
		switch (mensaje)
			{
			case 1:
				UartSendString(UART_PC, "Humedad baja, \r\n");
				UartSendString(UART_PC, "pH:\r\n");
				UartSendString(UART_PC, (char*)UartItoa(data, 10));
				UartSendString(UART_PC, " \r");

				break;
			case 2:
				UartSendString(UART_PC, "Humedad correcta, \r\n");
				UartSendString(UART_PC, "pH:\r\n");
				UartSendString(UART_PC, (char*)UartItoa(data, 10));
				UartSendString(UART_PC, " \r");

				break;
			case 3:
				UartSendString(UART_PC, "Bomba pHA encendida\r\n");
				UartSendString(UART_PC, " \r");

				break;
			case 4:
				UartSendString(UART_PC, "Bomba pHB encendida\r\n");
				UartSendString(UART_PC, " \r");

				break;
			case 5:
				UartSendString(UART_PC, "Bomba de agua encendida\r\n");
				UartSendString(UART_PC, " \r");

				break;
			case 6: 
				UartSendString(UART_PC, "pH: \r\n");
				UartSendString(UART_PC, (char*)UartItoa(data, 10));
				UartSendString(UART_PC, " \r");

				break;

			default:
				break;
			}
	}
	
}

void LeerTeclas()
{
	while(1)
	{
     	teclas  = SwitchesRead();
     	switch(teclas){
     		case SWITCH_1:
				encendido = true;
			break;
			case SWITCH_2:
				encendido = false;
			break;
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);
	}
}

void app_main(void)
{
	GPIODeinit();

	analog_input_config_t analog = 
	{
		.input = CH1, // Le paso el canal
		.mode = ADC_SINGLE, // El modo en el que va a operar
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&analog);
	AnalogOutputInit();

	serial_config_t my_uart = {
		.port = UART_PC, 
		.baud_rate = 115200, 
		.func_p = NULL, 
		.param_p = NULL
	};
	UartInit(&my_uart);

	timer_config_t timer_1 = {
			.timer = TIMER_A,
			.period = CONFIG_BLINK_PERIOD_SENSADO,
			.func_p = FuncTimer,
			.param_p = NULL
	};
	TimerInit(&timer_1);

	timer_config_t timer_2 = {
			.timer = TIMER_B,
			.period = CONFIG_BLINK_PERIOD_INFORME,
			.func_p = FuncTimerOut,
			.param_p = NULL
	};
	TimerInit(&timer_2);

	xTaskCreate(&SensarHumedadTask, "SensarHumedadTask", 512, NULL, 4, &sensarhumedadtask_task_handle);
	xTaskCreate(&SensarPHTask, "SensarPHTask", 512, NULL, 4, &sensarphtask_task_handle);
	xTaskCreate(&MostrarMensajeTask, "MostrarMensajeTask", 512, NULL, 4, &mostrarmensajetask_task_handle);

	TimerStart(timer_1.timer);
	TimerStart(timer_2.timer);	
}
/*==================[end of file]============================================*/