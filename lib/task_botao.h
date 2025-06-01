#ifndef TASK_BOTAO_H
#define TASK_BOTAO_H

#define BOTAO_PIN 5

volatile bool estadoAnteriorBotao = true;

void vBotaoTask() {
    gpio_init(BOTAO_PIN);
    gpio_set_dir(BOTAO_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_PIN);

    while (1) {
        bool estadoAtual = gpio_get(BOTAO_PIN);
        if (!estadoAtual && estadoAnteriorBotao) {
            desativarAlarme = true;
            printf("Alarme desativado!\n");
        }
        estadoAnteriorBotao = estadoAtual;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#endif