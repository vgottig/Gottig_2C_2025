/*! @mainpage Examen final 1° llamado 
 *
 * @section genDesc General Description
 *
 * Diseño de un dispositivo basado en la ESP-EDU que permita detectar eventos peligrosos para ciclistas.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |   ESP32      | Periferico        |
 * |:------------:|:---------------|
 * | GND          | GND	           | 
 * | GPIO_6       | Buzzer	       | 
 * | GPIO_3       | ECHO           |
 * | GPIO_2       | TRIGGER        |
 * | GND          | GND	           |
 * | CH0          | Acelerometro X |
 * | CH1          | Acelerometro Y |
 * | CH           | Acelerometro Z |
 * | BT (UART) TX | GPIO_18	       |
 * | BT	(UART) RX |	GPIO_19	       |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 02/12/2025 | Document creation		                         |
 *
 * @author Valentina Gottig (valentinagottig@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "hc_sr04.h"
#include "led.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_medicion_US 500000
#define CONFIG_BLINK_PERIOD_caida_US 10000 // 100Hz pasado a seg es 0.01s que son 10ms -> 10000us
#define Buzzer GPIO_6 // Alarma sonora

/*==================[internal data definition]===============================*/
uint16_t distancia_vehiculos;
TaskHandle_t deteccion_handle = NULL;
TaskHandle_t deteccion_caidas_handle = NULL;
uint16_t umbral_caida = 2850; // Se calcula como 4*0.3V + 1.65V = 2,85V => El valor dado por el conversor DA es en mV entonces -> 2850 mV
uint16_t lectura_eje_X;
uint16_t lectura_eje_y;
uint16_t lectura_eje_z;
uint16_t sumatoria_ejes;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Callback del timer que notifica a la tarea que detecta la presencia de vehículos.
 * @param param Puntero a parámetros (no usado).
 */
void TimerPresenciaHandler(void *param)
{
	vTaskNotifyGiveFromISR(deteccion_handle, pdFALSE);
}

/**
 * @brief Callback del timer que notifica a la tarea que detecta una caída.
 * @param param Puntero a parámetros (no usado).
 */
void TimerCaidasHandler(void *param)
{
	vTaskNotifyGiveFromISR(deteccion_caidas_handle, pdFALSE);
}

/**
 * @brief  Lee el valor del sensor de HC-SR04 y lo compara con un valor establecido. Si
 * supera ese valor prende los leds de acuerdo a la proximidad. A su vez hace sonar un buzzer con distinta frecuencia 
 * depende que tan cerca se encuentre el vehículo. Tambien envia por UART a un dispositivo bluetooth un mensaje de alerta
 * dependiendo la distancia a la que se encuentre el vehiculo.
 * @param pvParameter Puntero a parámetros (no usado).
 */
static void medicion_vehiculos(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Espera la notificación del timer 

		distancia_vehiculos = HcSr04ReadDistanceInCentimeters();

		distancia_vehiculos = distancia_vehiculos * 100; // Convierto los valores a metros

		if (distancia_vehiculos > 5)
			LedOn(LED_1); // LED verde

		else

			LedsOffAll();

		if (distancia_vehiculos < 5 && distancia_vehiculos > 3)
		{
			LedOn(LED_1); // LED verde y
			LedOn(LED_2); // LED amarillo
			UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca "); // UART_CONNECTOR es un UART externo por los pines GPIO18 y GPIO19
			UartSendString(UART_CONNECTOR, "\r\n");

			while (distancia_vehiculos < 5 && distancia_vehiculos > 3)
			{
				GPIOOn(Buzzer);
				vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo
				GPIOOff(Buzzer);
			}
		}
		else
			LedsOffAll();

		if (distancia_vehiculos < 3)
		{
			LedOn(LED_1); // LED verde
			LedOn(LED_2); // LED amarillo y 
			LedOn(LED_3); // LED rojo
			UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca");
			UartSendString(UART_CONNECTOR, "\r\n");

			while (distancia_vehiculos < 3)
			{
				GPIOOn(Buzzer);
				vTaskDelay(pdMS_TO_TICKS(500)); // Espera 0.5 segundos
				GPIOOff(Buzzer);
			}
		}
		else
			LedsOffAll();
	}
}

/**
 * @brief  Lee el valor del acelerometro en cada eje y calcula la aceleración que va a tener en base a
 * una suma escalar. A su vez manda por UART al dispositivo bluetooth un mensaje de si se detecta una caída 
 * @param pvParameter Puntero a parámetros (no usado).
 */
static void detector_caidas(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		   AnalogInputReadSingle(CH0, &lectura_eje_X);
		   AnalogInputReadSingle(CH1, &lectura_eje_y);
		   AnalogInputReadSingle(CH2, &lectura_eje_z);

		sumatoria_ejes = lectura_eje_X + lectura_eje_y + lectura_eje_z;
		
		if(sumatoria_ejes > umbral_caida)
		{
			UartSendString(UART_CONNECTOR, "Caída detectada");
			UartSendString(UART_CONNECTOR, "\r\n");
		}

	}
}

/*==================[external functions definition]==========================*/
void app_main(void){

	/*Se inicializa el sensor de ultrasonido*/
	HcSr04Init(GPIO_3, GPIO_2);

	/*Se inicializan los leds*/
	LedsInit();

    /*Se inicializa el GPIO*/
	GPIOInit(Buzzer, GPIO_OUTPUT);

	/*Se configura la uart*/
	serial_config_t my_uart = {
		.port = UART_CONNECTOR,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&my_uart);

	/*Se configuran los ADC*/
	analog_input_config_t X = {
		.input = CH0,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL,
	};
	AnalogInputInit(&X);

		analog_input_config_t Y = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL,
	};
	AnalogInputInit(&Y);

		analog_input_config_t Z = {
		.input = CH2,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL,
	};
	AnalogInputInit(&Z);

	/*Se configuran los timers*/
		timer_config_t timer_deteccion = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_medicion_US,
		.func_p = TimerPresenciaHandler,
		.param_p = NULL
	};
	TimerInit(&timer_deteccion);

		timer_config_t timer_caida = {
		.timer = TIMER_C,
		.period = CONFIG_BLINK_PERIOD_caida_US,
		.func_p = TimerCaidasHandler,
		.param_p = NULL
	};
	TimerInit(&timer_caida);

	/*Creación de tareas*/
	xTaskCreate(&medicion_vehiculos, "Sensado distancia vehiculos", 2048, NULL, 5, &deteccion_handle);
	xTaskCreate(&detector_caidas, "Detecta si hubo caída", 2048, NULL, 5, &deteccion_caidas_handle);

	/*Se inician los timers*/
	TimerStart(timer_deteccion.timer);
	TimerStart(timer_caida.timer);

}
/*==================[end of file]============================================*/