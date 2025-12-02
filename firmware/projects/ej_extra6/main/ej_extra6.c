/*! @mainpage Práctica
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral    |   ESP32   	            |
 * |:----------------:|:------------------------|
 * |HcSr04 (tanque 1) | GPIO_20 y GPIO_21		|
 * |HcSr04 (tanque 2) | GPIO_22 y GPIO_23		|
 * |     BOMBA 1      |          GPIO_8	    	|
 * |     BOMBA 2      |          GPIO_9 	   	|
 * | SENSOR PRESION   |            CH1 	   	    |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
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
#include "hc_sr04.h" 
#include "gpio_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "led.h"
#include "lcditse0803.h" 
#include "switch.h"


#define TIME_PERIOD1 500000 // 2 veces por seg -> cada 0.5 seg
#define TIME_PERIOD2 250000 // 4 veces por seg -> cada 0.25 seg


#define BOMBA1 GPIO_8
#define BOMBA2 GPIO_9
#define SENSOR_PRESION CH1

#define SENSIBILIDAD 730.0 //(mV/bar)

/*==================[macros and definitions]=================================*/
uint16_t distancia1;
uint16_t distancia2;
uint16_t litros1;
uint16_t litros2;
/*==================[internal data definition]===============================*/
TaskHandle_t task_handle1 = NULL;
TaskHandle_t task_handle2 = NULL;
TaskHandle_t task_handle3 = NULL;


bool tecla4 = false;
/*==================[internal functions declaration]=========================*/
/** @fn  void Notify(void *param)
 * @brief  notifica a la tareas task_handle1
 * @param *param
 */
void Notify1(void *param)
{
    vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
}
/** @fn  void Notify(void *param)
 * @brief  notifica a la tareas task_handle2
 * @param *param
 */
void Notify2(void *param)
{
    vTaskNotifyGiveFromISR(task_handle2, pdFALSE);
}
/** @fn  uint16_t convertirDistanciaAVolumen_tanque1(uint16_t distancia1)
 * @brief  convierte la distancia media en el tanque 1 en volumen
 * @param .uint16_t distancia1
 */
uint16_t convertirDistanciaAVolumen_tanque1(uint16_t distancia1)
{
    uint16_t alturaAgua1;
    uint16_t volumen1;
    uint16_t litros1;

    alturaAgua1 = 30 - distancia1;
    volumen1 = alturaAgua1 * 30 * 30;
    litros1 = volumen1 * 1 / 1000;

    return litros1;
}
/** @fn  uint16_t convertirDistanciaAVolumen_tanque2(uint16_t distancia2)
 * @brief  convierte la distancia media en el tanque 2 en volumen
 * @param .uint16_t distancia2
 */
uint16_t convertirDistanciaAVolumen_tanque2(uint16_t distancia2)
{
    uint16_t alturaAgua2;
    uint16_t volumen2;
    uint16_t litros2;

    alturaAgua2 = 30 - distancia2;
    volumen2 = alturaAgua2 * 30 * 30;
    litros2 = volumen2 * 1 / 1000;

    return litros2;
}

/** @fn  void duchar()
 * @brief  activa la bomba1 que se encarga de impulsar el agua desde el tanque1 a la zona de baño
 */
void duchar()
{
    GPIOOn(BOMBA1);
}

/** @fn  void noDuchar()
 * @brief  desactiva la bomba1 que se encarga de impulsar el agua desde el tanque1 a la zona de baño
 */
void noDuchar()
{
    GPIOOff(BOMBA1);
}

/** @fn  void recuperarAgua()
 * @brief  activa la bomba2 que se encarga de recuperar el agua usada y la deposita en el tanque_2.
 */
void recuperarAgua()
{
    GPIOOn(BOMBA2);
}

/** @fn  void noRecuperarAgua()
 * @brief  desactiva la bomba2 que se encarga de recuperar el agua usada y la deposita en el tanque_2.
 */
void noRecuperarAgua()
{
    GPIOOff(BOMBA2);
}

uint16_t ConvertirVoltajeAPresion(uint16_t presion_mV)
{
    uint16_t presion_bar = presion_mV / SENSIBILIDAD;
    return presion_bar;
}

void FunctionOnOff()
{
    tecla4 = !tecla4;
}

void Task_controlarVolAguaTanques(void *pvParameter)
{
    uint16_t distancia1;
    uint16_t distancia2;
    uint16_t litros1;
    uint16_t litros2;
    while (1)
    {
       ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // la tarea espera hasta que se reciba una notificación.
       if(tecla4)
       {
        HcSr04Init(GPIO_20, GPIO_21); // distancia tanque 1
        distancia1 = HcSr04ReadDistanceInCentimeters();

        HcSr04Init(GPIO_22, GPIO_23); // distancia tanque 2
        distancia2 = HcSr04ReadDistanceInCentimeters();

        litros1 = convertirDistanciaAVolumen_tanque1(distancia1);
        litros2 = convertirDistanciaAVolumen_tanque2(distancia2);

        if (litros1 > 2)
        {
            if (litros2 < 25)
                {
                    duchar();
                }
        }
        else
            noDuchar();

        // MESAJES POR LA UART
        UartSendString(UART_PC, "Tanque de agua limpia\r\n");
        UartSendString(UART_PC, (char *) UartItoa(litros1, 10));
        UartSendString(UART_PC, "litros\r\n");

        UartSendString(UART_PC, "Tanque de agua sucia\r\n");
        UartSendString(UART_PC, (char *) UartItoa(litros2, 10));
        UartSendString(UART_PC, "litros\r\n");
    }
    }
}

void Task_duchar(void *pvParameter)
{
    uint16_t presion_mV;
    uint16_t presion_bar;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // la tarea espera hasta que se reciba una notificación.
        if(tecla4)
        {
        AnalogInputReadSingle(SENSOR_PRESION, &presion_mV); // Lee el valor de la presion en voltaje (mV)
        presion_bar = ConvertirVoltajeAPresion(presion_mV);


        if (presion_bar < 0.5) // 500mbar = 0.5 bar
        {
            duchar();
        }
        else
            noDuchar();
    }
    }
}

void Task_retroalimentacion(void *pvParameter)
{
    float tiempoDucha;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // la tarea espera hasta que se reciba una notificación.

        tiempoDucha = litros1 * 1 / 0.750;
        LcdItsE0803Write(tiempoDucha);   //muestro dato por display lcd

        if (litros1 < 2)
        {
            LedOn(LED_1);
        }
        if (litros2 > 25)
        {
            LedOn(LED_1);
        }
        if ((litros1 > 2) && (litros2 < 25))
        {
            LedOn(LED_3);
        }
        if (GPIORead(BOMBA1))
        {
            LedOn(LED_2);
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

    // configuro timer1 para control de volumen de agua en los tanques
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = TIME_PERIOD1,
        .func_p = Notify1,
        .param_p = NULL};

    TimerInit(&timer_1); // inicializo timer 1

    timer_config_t timer_2 = { // configuro timer 2 para el control de la ducha
    .timer = TIMER_B,
    .period = TIME_PERIOD2,
    .func_p = Notify2,
    .param_p = NULL};

    TimerInit(&timer_2); // inicializo timer 2

    TimerStart(timer_1.timer); // para que comience el timer 1
    TimerStart(timer_2.timer); // para que comience el timer 2


    GPIOInit(BOMBA1, GPIO_OUTPUT);
    GPIOInit(BOMBA2, GPIO_OUTPUT);


    // configuracion para entrada analogica
    analog_input_config_t analogInput1 = {
        .input = CH1,       // Se configura para leer del canal 1 del conversor analógico-digital (ADC)
        .mode = ADC_SINGLE, // Se configura para realizar una única lectura analógica
    };
    AnalogInputInit(&analogInput1); // Inicializa


    // Para la uart
    serial_config_t my_uart = {
        .port = UART_PC,
        .baud_rate = 115200, /*!< baudrate (bits per second) */
        .func_p = NULL,      /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
        .param_p = NULL      /*!< Pointer to callback function parameters */
    };
    UartInit(&my_uart);


    SwitchesInit();

    // ESTO CON INTERRUPCIONES
    SwitchActivInt(SWITCH_1, &FunctionOnOff, NULL);  //puse tecla 1 en vez de 4

//creo tareas
    xTaskCreate(&Task_controlarVolAguaTanques, "controlar el volumen de agua en los tanques", 512, NULL, 5, &task_handle1);
    xTaskCreate(&Task_duchar, "duchar", 512, NULL, 5, &task_handle2);
    xTaskCreate(&Task_retroalimentacion, "distancias_task", 512, NULL, 5, &task_handle3);
}
/*==================[end of file]============================================*/