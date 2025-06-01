#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lib/bootsel_btn.h"

// VARIÁVEIS GLOBAIS
volatile bool desativarAlarme = false;
volatile bool alarmeAtivo = false;
volatile bool condicaoCritica = false;

// TASKS LIBS
#include "lib/task_sensor.h"
#include "lib/task_DHT.h"
#include "lib/task_buzzer.h"
#include "lib/task_botao.h"
#include "lib/task_mqtt.h"
#include "lib/task_display.h"

// FUNÇÃO PRINCIPAL
int main() {
    stdio_init_all();
    
    bootsel_btn_callback(); // Para ser utilizado o modo BOOTSEL com botão B
    
    xTaskCreate(vSensorTask, "Sensor", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vTaskDHT, "DHT22", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vAlarmeTask, "Alarme", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBotaoTask, "Botao", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vTaskMQTTPrates, "MQTT", configMINIMAL_STACK_SIZE * 6, NULL, tskIDLE_PRIORITY + 2, NULL);

    vTaskStartScheduler();
}