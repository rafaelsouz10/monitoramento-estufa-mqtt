# Estufa Inteligente com Monitoramento e Alarme via protocolo MQTT 

---

## Descrição do Projeto
Sistema embarcado de monitoramento ambiental com foco em estufas agrícolas. Utiliza a placa Raspberry Pi Pico W com FreeRTOS para leitura de sensores, acionamento de alarme, exibição em display OLED e comunicação com broker MQTT. O sistema permite visualização remota e controle de alarme via aplicativo IoT MQTT Panel.

---

## Requisitos

- **Microcontrolador**: Raspberry Pi Pico W
- **Editor de Código**: Visual Studio Code (VS Code)
- **SDK do Raspberry Pi Pico** devidamente configurado
- **Ferramentas de build**: CMake e Ninja
- **Broker MQTT local** (ex: Mosquitto) ou público (ex: test.mosquitto.org)
- Aplicativo Android: **IoT MQTT Panel**

---

## Instruções de Uso

### 1. Clone o Repositório
```bash
git clone https://github.com/rafaelsouz10/monitoramento-estufa-mqtt.git
cd monitoramento-estufa-mqtt
code .
```

---

### 2. Instale as Dependências

### 2.1 Certifique-se de que o SDK do Raspberry Pi Pico está configurado corretamente no VS Code. As extensões recomendadas são:

- **C/C++** (Microsoft).
- **CMake Tools**.
- **Raspberry Pi Pico**.

### 2.1 Configure o FreeRTOS (manual)

Para utilizar o FreeRTOS com a Raspberry Pi Pico, você deve fazer a integração manual do kernel ao seu projeto.

**Passo a passo:**

1. Baixe o kernel do FreeRTOS (ou clone do GitHub):
   ```bash
   git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git

2. Defina o caminho do kernel no seu CMakeLists.txt na linha 31:

    ```CMakeLists.txt
    set(FREERTOS_KERNEL_PATH "E:/CAMINHO-DO-KERNEL-BAIXADO/FreeRTOS-Kernel")

---

### 3. Configure o VS Code

Abra o projeto no Visual Studio Code e siga as etapas abaixo:

1. Certifique-se de que as extensões mencionadas anteriormente estão instaladas.
2. No terminal do VS Code, compile o código clicando em "Compile Project" na interface da extensão do Raspberry Pi Pico.
3. O processo gerará o arquivo `.uf2` necessário para a execução no hardware real.

---

### 4. Teste no Hardware Real

#### Utilizando a Placa de Desenvolvimento BitDogLab com Raspberry Pi Pico W:

1. Conecte a placa ao computador no modo BOTSEEL:
   - Pressione o botão **BOOTSEL** (localizado na parte de trás da placa de desenvolvimento) e, em seguida, o botão **RESET** (localizado na frente da placa).
   - Após alguns segundos, solte o botão **RESET** e, logo depois, solte o botão **BOOTSEL**.
   - A placa entrará no modo de programação.

2. Compile o projeto no VS Code utilizando a extensão do [Raspberry Pi Pico W](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico):
   - Clique em **Compile Project**.

3. Execute o projeto clicando em **Run Project USB**, localizado abaixo do botão **Compile Project**.

---

### 🔌 5. Conexões e Esquemático
Abaixo está o mapeamento de conexões entre os componentes e a Raspberry Pi Pico W:

| **Componentes**        | **Pino Conectado (GPIO)** |
|------------------------|---------------------------|
| Display SSD1306 (SDA)  | GPIO 14                   |
| Display SSD1306 (SCL)  | GPIO 15                   |
| Potênciometro Joystick | GPIO 27                   |
| LED RGB Vermelho       | GPIO 13                   |
| LED RGB Azul           | GPIO 12                   |
| LED RGB Verde          | GPIO 11                   |
| Buzzer                 | GPIO 21                   |
| Botão A                | GPIO 5                    |
| DHT22                  | GPIO 8                    |

#### 🛠️ Hardware Utilizado
- **Microcontrolador Raspberry Pi Pico W**
- **Display OLED SSD1306 (I2C)**
- **Botão A**
- **Buzzer**
- **LED RGB**
- **FreeRTOS**
- **Joystick**
- **Wi-Fi (CYW43439)**

---

### 📌 6. Funcionamento do Sistema

- Leitura da temperatura ambiente via **sensor DHT22**
- Simulação de temperatura com **potenciômetro** (entrada ADC) para testes
- Verificação de **condição crítica** com base na faixa segura entre **15°C e 40°C**
- Acionamento automático do **buzzer** em caso de alerta (temperatura fora da faixa)
- Desativação do alarme:
  - Por **botão físico** (GPIO 5)
  - Ou remotamente via **aplicativo MQTT** publicando `OFF` no tópico `/controle/alarme`
- Publicação periódica dos dados nos seguintes **tópicos MQTT**:
  - `/temp_dht` → temperatura ambiente (sensor DHT22)
  - `/umi_dht` → umidade do ar (sensor DHT22)
  - `/temp_pot` → temperatura simulada pelo potenciômetro
  - `/estado` → indica "OK" ou "CRITICO"
  - `/alarme` → indica "ON" ou "OFF"
- Exibição de todos os dados no **display OLED SSD1306**
- Estado da conexão MQTT com o broker também é exibido:
  - `Broker ON` ou `Broker OFF`

---

### 7. Broker MQTT e Aplicativo

- Broker utilizado: **Mosquitto** rodando localmente
- Aplicativo Android utilizado: **IoT MQTT Panel**
  - O botão de controle envia a mensagem `OFF` para o tópico `/controle/alarme` com QoS 1
  - Medidores e gráficos são configurados para os tópicos:
    - `/temp_dht`, `/umi_dht`, `/temp_pot`
    - `/estado` (texto)
    - `/alarme` (indicador visual ON/OFF)
- O painel permite monitorar a estufa em tempo real e desligar o alarme remotamente

---

### 8. Vídeos Demonstrativo

**Click [AQUI](https://drive.google.com/file/d/1n5OE0JvYYrGcrIpVdzTJf6uPoj5HJ7dK/view?usp=sharing) para acessar o link do Vídeo Ensaio**