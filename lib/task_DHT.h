//reaproveitado do repositório de Valentin Milea: https://github.com/vmilea/pico_dht

#ifndef TASK_DHT_H
#define TASK_DHT_H

#include "dht.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Configurações
static const dht_model_t DHT_MODEL = DHT22;
static const uint DATA_PIN = 8;

// Variáveis globais para uso no display/webserver
volatile float tempDHT = 0.0;
volatile float umiDHT = 0.0;

// Task para FreeRTOS
static inline void vTaskDHT(void *pvParameters) {
    dht_t dht;
    dht_init(&dht, DHT_MODEL, pio0, DATA_PIN, true /* pull_up */);

    while (1) {
        dht_start_measurement(&dht);

        float humidity = 0.0;
        float temperature_c = 0.0;

        dht_result_t result = dht_finish_measurement_blocking(&dht, &humidity, &temperature_c);

        if (result == DHT_RESULT_OK) {
            tempDHT = temperature_c;
            umiDHT = humidity;

            // printf("[DHT OK] Temp: %.1fC  %.1f%% humidity\n", tempDHT, umiDHT);
        } else if (result == DHT_RESULT_TIMEOUT) {
            puts("[DHT FAIL] Sensor não respondeu. Cheque fiação.");
        } else if (result == DHT_RESULT_BAD_CHECKSUM) {
            puts("[DHT FAIL] Checksum incorreto.");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay no estilo FreeRTOS
    }
}

#endif // TASK_DHT_H
