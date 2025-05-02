# WebServer Pico W - Monitoramento de Sensores

Este projeto implementa um servidor web em um **Raspberry Pi Pico W**, permitindo o monitoramento em tempo real de sensores analógicos (joystick), um microfone e um botão físico via Wi-Fi. Os dados são exibidos em uma interface web acessível pela rede local.

## 🖥️ Funcionalidades

- Acesso via navegador à página HTML hospedada pelo Pico W.
- Leitura contínua de:
  - Valor analógico do microfone.
  - Posição X e Y do joystick.
  - Estado de um botão físico (pressionado ou solto).
- Interface web dinâmica com atualizações a cada segundo via `fetch`.

## 📦 Componentes utilizados

- Raspberry Pi Pico W
- Módulo joystick analógico (conectado aos canais ADC 0 e 1)
- Microfone analógico (conectado ao canal ADC 2 / GPIO 28)
- Botão físico (conectado ao GPIO 5)
- LEDs indicativos (opcional):
  - LED azul: GPIO 12
  - LED verde: GPIO 11
  - LED vermelho: GPIO 13

## 🚀 Como usar

### 1. Clone este repositório:
```bash
git clone https://github.com/antoniojosemota/webserverpico.git
cd webserverpico
```
A estrutura deve ficar assim:
```bash
/webserverpico
├── CMakeLists.txt
├── main.c
└── README.md
```

### 2. Configure o ambiente 
Usando o SDK do Raspberry Pi Pico.
Certifique-se de que você tem as dependências para:
- pico-sdk
- lwIP (incluído no SDK do Pico W)
- cyw43-driver (Wi-Fi)
### 3. Substitua as variáveis do Wi-Fi
```bash
#define WIFI_SSID "Seu_SSID"
#define WIFI_PASSWORD "Sua_Senha"
```
Após essa etapa, compile e envie os dados para a placa.
### 4. Conecte-se e acesse a página
Após o boot, o IP do dispositivo será exibido no terminal (via printf). Acesse esse IP pelo navegador:
```bash
http://<endereço_ip_do_pico>
```
A página será exibida com os valores dos sensores sendo atualizados automaticamente.
### 5. Sobre o funcionamento
- Leitura dos eixos X e Y do joystick (ADC0 e ADC1)
- Leitura do microfone (ADC2, normalizado para 0–100)
- Leitura do botão (GPIO com pull-up)
- Atualização automática dos dados via JavaScript (fetch a cada 1s)
- O servidor TCP é configurado na porta 80.

### Projetado por Antonio José Mota
