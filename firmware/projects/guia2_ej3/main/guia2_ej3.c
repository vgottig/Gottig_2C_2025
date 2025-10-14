/*! @mainpage Guia 2. Ejercicio 3
 *
 * @section genDesc General Description
 *
 * Proyecto: Medidor de distancia por ultrasonido c/interrupciones y puerto serie
 * 
 * Cree un nuevo proyecto en el que modifique la actividad del punto 2 agregando
 * ahora el puerto serie. Envíe los datos de las mediciones para poder observarlos
 * en un terminal en la PC, siguiendo el siguiente formato:
 * 
 * ● 3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
 * 
 * Además debe ser posible controlar la EDU-ESP de la siguiente manera:
 * 
 * ● Con las teclas “o” y “h”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
 * 
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
 * | 01/10/2025 | Document creation		                         |
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
#include "led.h"
#include "switch.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "hc_sr04.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define REFRESH_PERIOD_US 1000000   // 1 segundo
#define UART_NO_INT 0
#define UART_BAUDRATE 115200

/*==================[internal data definition]===============================*/
bool tecla1 = false;   // Control de medición (ON/OFF)
bool tecla2 = false;   // Control de HOLD
TaskHandle_t dist_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Handler de interrupción de la tecla 1 (TEC1)
 */
void Tecla1Handler(){
    tecla1 = !tecla1;   // Activa o detiene la medición
}

/**
 * @brief Handler de interrupción de la tecla 2 (TEC2)
 */
void Tecla2Handler(){
    tecla2 = !tecla2;   // Activa o desactiva el HOLD
}

/**
 * @brief Handler de interrupción del timer (cada 1s)
 */
void TimerDistHandler(void *param){
    vTaskNotifyGiveFromISR(dist_task_handle, pdFALSE);
}

/** 
 * @brief Handler de recepción por UART (control remoto con 'o' y 'h')
 */
void UartRxHandler(void *param){
    uint8_t data;

    UartReadByte(UART_PC, &data);

    switch(data){
        case 'o':
            Tecla1Handler();
            break;
        case 'h':
            Tecla2Handler();
            break;
    }
}

/**
 * @brief Tarea principal: mide distancia y controla LEDs + LCD
 */
void distanciasTask(void *pvParameter){
    uint8_t distancia;

    while(true){
        // Espera la notificación del timer
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if(tecla1){  // Si la medición está activada
            distancia = HcSr04ReadDistanceInCentimeters();

            if(!tecla2){   // Si no está en HOLD
                LcdItsE0803Write(distancia);
            }

            // Formato UART: 3 dígitos + espacio + "cm" + \r\n
            UartSendString(UART_PC, (char *)UartItoa(distancia, 10)); // Lo que mido, lo convierto a string y lo muestro en la PC
			UartSendString(UART_PC, " cm\n\r");

            // Control de LEDs según la distancia
            if(distancia < 10){
                LedsOffAll();
            }
            else if(distancia >= 10 && distancia < 20){
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if(distancia >= 20 && distancia < 30){
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }
            else if(distancia >= 30){
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
        }else{
            // Si no está activa la medición
            LcdItsE0803Off();
            LedsOffAll();
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
    // Inicialización de periféricos
    LedsInit();
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2);
    SwitchesInit();

    // Configuración de interrupciones de teclas
    SwitchActivInt(SWITCH_1, &Tecla1Handler, NULL);
    SwitchActivInt(SWITCH_2, &Tecla2Handler, NULL);

    // Configuración de timer para refresco cada 1s
    timer_config_t timer_dist = {
        .timer = TIMER_A,
        .period = REFRESH_PERIOD_US,
        .func_p = TimerDistHandler,
        .param_p = NULL
    };
    TimerInit(&timer_dist);

    serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = UART_BAUDRATE,
		.func_p = UartRxHandler,
		.param_p = NULL};
	UartInit(&my_uart);

    // Creación de la tarea de medición
    xTaskCreate(&distanciasTask, "dist_task", 512, NULL, 5, &dist_task_handle);

    // Arranque del timer
    TimerStart(timer_dist.timer);
}

/*==================[end of file]============================================*/