#ifndef TASK_BUZZER_H
#define TASK_BUZZER_H

#include "hardware/structs/timer.h" // Para usar alarmes via add_alarm_in_us

#define BUZZER 21

// Variáveis de controle
volatile bool buzzer_estado = false; // Controla do estado atual do buzzer (ligado ou desligado)
alarm_id_t buzzer_alarm_id = -1;    // Armazena o ID do alarm ativo (usado para cancelar depois)

// Callback do alarme: alterna o buzzer a cada 2000us (simula ~500Hz)
int64_t buzzer_alarm_callback(alarm_id_t id, void *user_data) {
    buzzer_estado = !buzzer_estado;
    gpio_put(BUZZER, buzzer_estado);
    return 2000; // Reexecuta o callback a cada 2ms (500Hz)
}

// Inicia o efeito sonoro do buzzer (se não estiver tocando).
void buzzer_start_alarm() {
    if (buzzer_alarm_id < 0) {  // Só agenda se não tiver um alarm ativo
        buzzer_alarm_id = add_alarm_in_us(2000, buzzer_alarm_callback, NULL, true);
    }
}

// Para o efeito sonoro do buzzer (se estiver tocando).
void buzzer_stop_alarm() {
    if (buzzer_alarm_id >= 0) {         // Só cancela se um alarm estiver ativo
        cancel_alarm(buzzer_alarm_id); // Cancela o alarme
        buzzer_alarm_id = -1;         // Reseta o ID
        gpio_put(BUZZER, 0);         // Desliga fisicamente o buzzer
        buzzer_estado = false;      // Reseta o estado de controle
    }
}

// TASK DO BUZZER 
void vAlarmeTask() {
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);

    while (1) {
        condicaoCritica = (temperatura < LIMITE_BAIXO || temperatura > LIMITE_ALTO);

        if (!desativarAlarme && condicaoCritica) {
            if (!alarmeAtivo) printf("Alarme ativado\n");
            alarmeAtivo = true; 
            buzzer_start_alarm();
            vTaskDelay(pdMS_TO_TICKS(150));
            buzzer_stop_alarm();
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            buzzer_stop_alarm();
            if (alarmeAtivo) printf("Alarme desativado\n");
            alarmeAtivo = false;   
        }

        if (!condicaoCritica && desativarAlarme) {
            desativarAlarme = false;
            printf("Alarme rearmado automaticamente.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif
