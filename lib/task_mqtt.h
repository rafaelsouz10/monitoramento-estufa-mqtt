#ifndef TASK_MQTT_H
#define TASK_MQTT_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "pico/unique_id.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/dns.h"
#include "lwip/altcp_tls.h"

// Configurações de rede e MQTT
#define WIFI_SSID "Kira_Oreo"
#define WIFI_PASS "Aaik1987"
#define MQTT_SERVER "192.168.0.122"
#define MQTT_USERNAME "admin_rafael"
#define MQTT_PASSWORD "rafael1234"
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

// Estrutura do cliente MQTT
typedef struct {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    ip_addr_t mqtt_server_address;
    bool connect_done;
    char topic[MQTT_TOPIC_LEN];
} MQTT_CLIENT_DATA_T;

// Prototipações
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

    // Verifica se é comando para desativar o alarme
    if (strcmp(state->topic, "/controle/alarme") == 0 && strcmp(msg, "OFF") == 0) {
        desativarAlarme = true;
        INFO_printf("→ Alarme desativado via MQTT.\n");
    }
}

// Callback chamado após tentativa de conexão com o broker
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        state->connect_done = true;
        INFO_printf("Conectado ao broker MQTT com sucesso!\n");

        mqtt_subscribe(client, "/controle/alarme", 1, NULL, NULL);
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, state);
    } else {
        state->connect_done = false;
        ERROR_printf("Falha ao conectar ao broker: %d\n", status);
        vTaskDelay(pdMS_TO_TICKS(5000));
        reconnect_mqtt_client(state);
    }
}

// Inicia o cliente MQTT
static void start_client(MQTT_CLIENT_DATA_T *state) {
    state->mqtt_client_inst = mqtt_client_new();
    if (!state->mqtt_client_inst) panic("Erro ao criar cliente MQTT");

    cyw43_arch_lwip_begin();
    if (mqtt_client_connect(state->mqtt_client_inst, &state->mqtt_server_address, MQTT_PORT, mqtt_connection_cb, state, &state->mqtt_client_info) != ERR_OK) {
        panic("Erro ao conectar ao broker MQTT");
    }
    cyw43_arch_lwip_end();
}

// Função chamada após resolver o endereço do broker via DNS
static void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    MQTT_CLIENT_DATA_T *state = (MQTT_CLIENT_DATA_T*)arg;
    if (ipaddr) {
        state->mqtt_server_address = *ipaddr;
        start_client(state);
    } else {
        panic("Falha ao resolver DNS");
    }
}

// Tenta reconectar ao broker
static void reconnect_mqtt_client(MQTT_CLIENT_DATA_T *state) {
    cyw43_arch_lwip_begin();
    int err = dns_gethostbyname(MQTT_SERVER, &state->mqtt_server_address, dns_found, state);
    cyw43_arch_lwip_end();

    if (err == ERR_OK) {
        start_client(state);
    } else if (err != ERR_INPROGRESS) {
        ERROR_printf("Falha ao resolver DNS\n");
    }
}

// Task principal MQTT com loop de publicação dos dados
void vMqttTask(void *pvParameters) {
    INFO_printf("Iniciando task MQTT\n");

    static MQTT_CLIENT_DATA_T state;

    if (cyw43_arch_init()) panic("Falha ao iniciar CYW43");

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        panic("Falha ao conectar no Wi-Fi");
    }

    // Gera nome único para o cliente
    char unique_id_buf[5];
    pico_get_unique_board_id_string(unique_id_buf, sizeof(unique_id_buf));
    for (int i = 0; i < sizeof(unique_id_buf) - 1; i++) unique_id_buf[i] = tolower(unique_id_buf[i]);

    char client_id_buf[sizeof(MQTT_DEVICE_NAME) + sizeof(unique_id_buf) - 1];
    memcpy(client_id_buf, MQTT_DEVICE_NAME, sizeof(MQTT_DEVICE_NAME) - 1);
    memcpy(&client_id_buf[sizeof(MQTT_DEVICE_NAME) - 1], unique_id_buf, sizeof(unique_id_buf) - 1);
    client_id_buf[sizeof(client_id_buf) - 1] = 0;
    INFO_printf("Device name %s\n", client_id_buf);

    // Configura informações do cliente MQTT
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

    // Inicia a conexão MQTT
    reconnect_mqtt_client(&state);

    // Loop principal: publica os dados dos sensores periodicamente
    while (!state.connect_done || mqtt_client_is_connected(state.mqtt_client_inst)) {
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

    INFO_printf("Finalizando task MQTT\n");
    vTaskDelete(NULL);
}

#endif
