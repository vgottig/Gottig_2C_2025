/*! @mainpage Guía 2. Ejercicio 4
 *
 * @section genDesc General Description
 *
 * Proyecto: Osciloscopio
 * 
 * Diseñar e implementar una aplicación, basada en el driver analog_io_mcu.h y el
 * driver de transmisión serie uart_mcu.h, que digitalice una señal analógica y la
 * transmita a un graficador de puerto serie de la PC. Se debe tomar la entrada CH1
 * del conversor AD y la transmisión se debe realizar por la UART conectada al
 * puerto serie de la PC, en un formato compatible con un graficador por puerto
 * serie.
 * 
 * Sugerencias:
 * ● Disparar la conversión AD a través de una interrupción periódica de timer.
 * ● Utilice una frecuencia de muestreo de 500Hz.
 * ● Obtener los datos en una variable que le permita almacenar todos los bits del conversor.
 * ● Transmitir los datos por la UART en formato ASCII a una velocidad de transmisión
 * suficiente para realizar conversiones a la frecuencia requerida.
 * 
 * Será necesario utilizar un graficador serie desde la PC. Se recomienda utilizar
 * la extensión de VSCode: “Serial Plotter”.
 * Formato de envío:
 * >brightness:234,temp:25.7\r\n
 * >brightness:200\r\n
 * >brightness:12\r\n
 * Se ignoran en la graficación datos sin “>”
 * 
 * Prueba del osciloscopio:
 * Convierta una señal digital de un ECG (provista por la cátedra) en una señal
 * analógica y visualice esta señal utilizando el osciloscopio que acaba de implementar.
 * Se sugiere utilizar el potenciómetro para conectar la salida del DAC a la entrada CH1 del AD.
 * 
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
 * | 15/10/2025 | Document creation		                         |
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
#include "timer_mcu.h"
#include "switch.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define TIME_PERIOD 2000 // 500 Hz
#define TIME_PERIOD2 4000 // 250 Hz
#define BUFFER_SIZE 231
uint8_t dato = 0;

/*==================[internal data definition]===============================*/
TaskHandle_t task_handle1 = NULL;
TaskHandle_t task_handle2 = NULL;

const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/

/** 
 * @brief Handler de interrupción del timer 
 */ 
void notifyTask(void *param)
{
    vTaskNotifyGiveFromISR(task_handle1, pdFALSE); // ADC
    vTaskNotifyGiveFromISR(task_handle2, pdFALSE); // DAC
}

/**
 * @brief Tarea que lee el ADC y envía los datos por UART.
 */
static void adcTask(void *pvParameter)
{
    uint16_t valorDigitalizado;

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        AnalogInputReadSingle(CH1, &valorDigitalizado);

        UartSendString(UART_PC, (char *)UartItoa(valorDigitalizado, 10));
        UartSendString(UART_PC, (char *)"\r\n");
    }
}

/**
 * @brief Tarea que genera la señal ECG en el DAC 
 */ 
static void dacTask(void *pvParamete)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        AnalogOutputWrite(ecg[dato]); // Convierte de digital a analógico
        dato ++;
        
        if (dato == sizeof(ecg)) // Recorro los datos del ECG
            dato = 0;
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){

    // Inicialización UART
    serial_config_t my_uart = {
        .port = UART_PC,     
        .baud_rate = 115200, 
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&my_uart);

    // Inicialización ADC
    analog_input_config_t analogInput1 = {
        .input = CH1,
    };
    AnalogInputInit(&analogInput1); 
    
    // Inicialización DAC
    AnalogOutputInit(); // GPIO0//CH0

    // Inicialización de timers
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = TIME_PERIOD,
        .func_p = notifyTask,
        .param_p = NULL
    };
    TimerInit(&timer_1);

    timer_config_t timer_2 = {
        .timer = TIMER_B,
        .period = TIME_PERIOD2,
        .func_p = notifyTask,
        .param_p = NULL
    };
    TimerInit(&timer_2);
   
    xTaskCreate(&adcTask, "leer y enviar por UART", 2048, NULL, 5, &task_handle1); 
    xTaskCreate(&dacTask, "generar señal ecg", 2048, NULL, 5, &task_handle2);
    
    TimerStart(timer_1.timer);
    TimerStart(timer_2.timer);

}
/*==================[end of file]============================================*/