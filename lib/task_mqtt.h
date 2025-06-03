#ifndef TASK_MQTT_H
#define TASK_MQTT_H

#include "pico/stdlib.h"            // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "pico/cyw43_arch.h"        // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "pico/unique_id.h"         // Biblioteca com recursos para trabalhar com os pinos GPIO do Raspberry Pi Pico

#include "lwip/apps/mqtt.h"         // Biblioteca LWIP MQTT -  fornece funções e recursos para conexão MQTT
#include "lwip/apps/mqtt_priv.h"    // Biblioteca que fornece funções e recursos para Geração de Conexões
#include "lwip/dns.h"               // Biblioteca que fornece funções e recursos suporte DNS:
#include "lwip/altcp_tls.h"         // Biblioteca que fornece funções e recursos para conexões seguras usando TLS:

// Configurações de rede e MQTT
#define WIFI_SSID "Kira_Oreo"        // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASSWORD "Aaik1987"     // Substitua pela senha da sua rede Wi-Fi
#define MQTT_SERVER "192.168.0.122"  // Substitua pelo endereço do host - broker MQTT
#define MQTT_USERNAME "admin" // Nome de usuário do broker MQTT
#define MQTT_PASSWORD "rafael123"   // Senha do broker MQTT

#define MQTT_KEEP_ALIVE_S 60
#define MQTT_WILL_TOPIC "/online"
#define MQTT_WILL_MSG "0"
#define MQTT_WILL_QOS 1
#define MQTT_DEVICE_NAME "pico"
#define MQTT_TOPIC_LEN 100

#ifndef INFO_printf
#define INFO_printf printf
#endif

#ifndef ERROR_printf
#define ERROR_printf printf
#endif

volatile char connBroker[20] = "Conect. Broker";

// Estrutura que armazena as informações do cliente MQTT
typedef struct {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    ip_addr_t mqtt_server_address;
    bool connect_done;
    char topic[MQTT_TOPIC_LEN];
} MQTT_CLIENT_DATA_T;

// Prototipações de funções auxiliares
static void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);
static void reconnect_mqtt_client(MQTT_CLIENT_DATA_T *state);

// Callback chamado ao receber um novo tópico publicado
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    strncpy(state->topic, topic, sizeof(state->topic));
    INFO_printf("Publicação recebida no tópico: %s\n", topic);
}

// Callback chamado ao receber dados do tópico publicado
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    char msg[len + 1];
    memcpy(msg, data, len);
    msg[len] = '\0';
    INFO_printf("Dados recebidos - Tópico: %s | Mensagem: %s\n", state->topic, msg);

    if (strcmp(state->topic, "/controle/alarme") == 0 && strcmp(msg, "OFF") == 0) {
        desativarAlarme = true;
        INFO_printf("→ Alarme desativado via MQTT.\n");
    }
}

// Callback após tentativa de conexão
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        state->connect_done = true;
        strcpy((char *)connBroker, "Broker ON");
        INFO_printf("Conectado ao broker MQTT com sucesso!\n");
        mqtt_subscribe(client, "/controle/alarme", 1, NULL, NULL);
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, state);
    } else {
        state->connect_done = false;
        strcpy((char *)connBroker, "Broker OFF");
        ERROR_printf("Falha ao conectar ao broker: %d\n", status);
    }
}

// Inicializa o cliente MQTT
static void start_client(MQTT_CLIENT_DATA_T *state) {
    state->mqtt_client_inst = mqtt_client_new();
    if (!state->mqtt_client_inst) {
        ERROR_printf("Erro ao criar cliente MQTT\n");
        return;
    }
    cyw43_arch_lwip_begin();
    if (mqtt_client_connect(state->mqtt_client_inst, &state->mqtt_server_address, MQTT_PORT, mqtt_connection_cb, state, &state->mqtt_client_info) != ERR_OK) {
        ERROR_printf("Erro ao conectar ao broker MQTT\n");
    }
    cyw43_arch_lwip_end();
}

// Tenta reconectar ao broker sem usar DNS
static void reconnect_mqtt_client(MQTT_CLIENT_DATA_T *state) {
    ip4addr_aton(MQTT_SERVER, ip_2_ip4(&state->mqtt_server_address));

    while (!state->connect_done) {
        start_client(state);

        // Aguarda tentativa de conexão
        for (int i = 0; i < 10 && !state->connect_done; i++) {
            cyw43_arch_poll();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        if (!state->connect_done) {
            ERROR_printf("Tentando reconectar em 5 segundos...\n");
            strcpy((char *)connBroker, "Conect. Broker");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}

// dns_found permanece, caso deseje voltar ao uso de nomes de domínio futuramente
static void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    MQTT_CLIENT_DATA_T *state = (MQTT_CLIENT_DATA_T*)arg;
    if (ipaddr) {
        state->mqtt_server_address = *ipaddr;
        start_client(state);
    } else {
        ERROR_printf("Falha ao resolver DNS\n");
    }
}

// Task principal MQTT
void vMqttTask(void *pvParameters) {
    INFO_printf("Iniciando task MQTT\n");
    static MQTT_CLIENT_DATA_T state;

    if (cyw43_arch_init()) {
        ERROR_printf("Falha ao iniciar CYW43\n");
        vTaskDelete(NULL);
    }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        ERROR_printf("Falha ao conectar no Wi-Fi\n");
        vTaskDelete(NULL);
    }

    char unique_id_buf[5];
    pico_get_unique_board_id_string(unique_id_buf, sizeof(unique_id_buf));
    for (int i = 0; i < sizeof(unique_id_buf) - 1; i++) unique_id_buf[i] = tolower(unique_id_buf[i]);

    char client_id_buf[sizeof(MQTT_DEVICE_NAME) + sizeof(unique_id_buf) - 1];
    memcpy(client_id_buf, MQTT_DEVICE_NAME, sizeof(MQTT_DEVICE_NAME) - 1);
    memcpy(&client_id_buf[sizeof(MQTT_DEVICE_NAME) - 1], unique_id_buf, sizeof(unique_id_buf) - 1);
    client_id_buf[sizeof(client_id_buf) - 1] = 0;
    INFO_printf("Device name %s\n", client_id_buf);

    state.mqtt_client_info.client_id = client_id_buf;
    state.mqtt_client_info.keep_alive = MQTT_KEEP_ALIVE_S;
    state.mqtt_client_info.client_user = MQTT_USERNAME;
    state.mqtt_client_info.client_pass = MQTT_PASSWORD;

    static char will_topic[MQTT_TOPIC_LEN];
    strncpy(will_topic, MQTT_WILL_TOPIC, sizeof(will_topic));
    state.mqtt_client_info.will_topic = will_topic;
    state.mqtt_client_info.will_msg = MQTT_WILL_MSG;
    state.mqtt_client_info.will_qos = MQTT_WILL_QOS;
    state.mqtt_client_info.will_retain = true;

    while(1){
        reconnect_mqtt_client(&state);

        while (mqtt_client_is_connected(state.mqtt_client_inst)) {
            cyw43_arch_poll();

            char payload_dht[16];
            snprintf(payload_dht, sizeof(payload_dht), "%.1f", tempDHT);
            mqtt_publish(state.mqtt_client_inst, "/temp_dht", payload_dht, strlen(payload_dht), 1, 0, NULL, NULL);
            cyw43_arch_poll();

            char payload_umi[16];
            snprintf(payload_umi, sizeof(payload_umi), "%.1f", umiDHT);
            mqtt_publish(state.mqtt_client_inst, "/umi_dht", payload_umi, strlen(payload_umi), 1, 0, NULL, NULL);
            cyw43_arch_poll();

            char payload_pot[16];
            snprintf(payload_pot, sizeof(payload_pot), "%.2f", temperatura);
            mqtt_publish(state.mqtt_client_inst, "/temp_pot", payload_pot, strlen(payload_pot), 1, 0, NULL, NULL);
            cyw43_arch_poll();

            const char *estado = condicaoCritica ? "CRITICO" : "OK";
            mqtt_publish(state.mqtt_client_inst, "/estado", estado, strlen(estado), 1, 0, NULL, NULL);
            cyw43_arch_poll();

            const char *alarme = alarmeAtivo ? "ON" : "OFF";
            mqtt_publish(state.mqtt_client_inst, "/alarme", alarme, strlen(alarme), 1, 0, NULL, NULL);
            cyw43_arch_poll();

            INFO_printf("DHT: %s°C | UMI: %s%% | POT: %s°C | Alarme: %s | Estado: %s\n", payload_dht, payload_umi, payload_pot, alarme, estado);

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ERROR_printf("Conexão com o broker perdida. Reiniciando task...\n");
    }
}

#endif
