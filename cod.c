#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h" // Para acessar netif_default e IP

// Configurações de Wi-Fi
#define WIFI_SSID "ENZOBOOK 4142"
#define WIFI_PASSWORD "ENZOMELO10"

// Definição dos pinos dos LEDs
#define LED_PIN CYW43_WL_GPIO_LED_PIN
#define LED_BLUE_PIN 12  // GPIO12 - LED azul
#define LED_GREEN_PIN 11 // GPIO11 - LED verde
#define LED_RED_PIN 13   // GPIO13 - LED vermelho
#define MIC 28
#define BUTTON_PIN 5 // GPIO15 - Botão físico

int button_state = 0; // Variável para armazenar o estado do botão

char *bussola(uint x, uint y) {
    char *result;

    if (x < 1500 && y < 1500) {
        result = "SUDOESTE";
        printf("SUDOESTE\n");
    } else if (x > 3000 && y < 1500) {
        result = "SUDESTE";
        printf("SUDESTE\n");
    } else if (x < 1500 && y > 3000) {
        result = "NOROESTE";
        printf("NOROESTE\n");
    } else if (x > 3000 && y > 3000) {
        result = "NORDESTE";
        printf("NORDESTE\n");
    } else if (x < 1500) {
        result = "OESTE";
        printf("OESTE\n");
    } else if (x > 3000) {
        result = "LESTE";
        printf("LESTE\n");
    } else if (y < 1500) {
        result = "SUL";
        printf("SUL\n");
    } else if (y > 3000) {
        result = "NORTE";
        printf("NORTE\n");
    }
    else {
        result = "CENTRO";
        printf("CENTRO\n");
    }

    return result;
}

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    tcp_recved(tpcb,p->len);

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    // Leitura do joystick
    adc_select_input(0);
    uint16_t x_value = adc_read();
    adc_select_input(1);
    uint16_t yy_value = adc_read();

    char *direction = bussola(x_value, yy_value);

    bool current_button = gpio_get(BUTTON_PIN);  // Lê direto
    const char *button_states = current_button ? "Solto" : "Pressionado";

    adc_select_input(2);
    uint16_t mic = adc_read();
    mic = (mic * 100) / 4095; // Normaliza o valor para 0-100

    // Resposta JSON se for /data
    if (strstr(request, "GET /data") != NULL)
    {
        char json_body[128];
        snprintf(json_body, sizeof(json_body), "{\"mic\": %d, \"button\": \"%s\", \"x\": \"%s\"}", mic, button_states, direction);

        char json[256];
        snprintf(json, sizeof(json),
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n" 
            "Content-Type: application/json\r\n"
            "Cache-Control: no-cache\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s",
            (int)strlen(json_body), json_body);

        tcp_write(tpcb, json, strlen(json), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);

        tcp_close(tpcb);                             
        tcp_recv(tpcb, NULL);

        free(request);
        pbuf_free(p);
        return ERR_OK;
    }

    // Página HTML principal
    char html[2048];
    snprintf(html, sizeof(html),
             "HTTP/1.1 200 OK\r\n"
             "Connection: close\r\n" 
             "Content-Type: text/html\r\n"
             "Cache-Control: no-cache\r\n"
             "\r\n"
             "<!DOCTYPE html>\n"
             "<html>\n"
             "<head>\n"
             "<title>Joystick Monitor</title>\n"
             "<style>\n"
             "body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
             "h1 { font-size: 48px; }\n"
             ".data { font-size: 36px; margin-top: 20px; }\n"
             "</style>\n"
             "<script>\n"
             "function updateData() {\n"
             "  fetch('/data').then(r => r.json()).then(data => {\n"
             "    document.getElementById('mic').textContent = data.mic;\n"
             "    document.getElementById('button').textContent = data.button;\n"
             "    document.getElementById('x').textContent = data.x;\n"
             "  });\n"
             "}\n"
             "setInterval(updateData, 1000);\n"
             "window.onload = updateData;\n"
             "</script>\n"
             "</head>\n"
             "<body>\n"
             "<h1>Monitoramento</h1>\n"
             "<div class=\"data\">Microfone: <span id=\"mic\">-</span></div>\n"
             "<div class=\"data\">Estado do botão: <span id=\"button\">-</span></div>\n"
             "<div class=\"data\">Direçãoj: <span id=\"x\">-</span></div>\n"
             "</body>\n"
             "</html>\n");

    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    tcp_close(tpcb);
    tcp_recv(tpcb, NULL);
    
    free(request);
    pbuf_free(p);
    return ERR_OK;
}

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

void connect_server(){


}

// Função principal
int main()
{
    stdio_init_all();

    // Configuração dos LEDs como saída
    adc_init();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // Habilita o pull-up interno para o botão

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, false);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false);

    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, false);

    adc_gpio_init(MIC);
    adc_select_input(2);

    while (cyw43_arch_init())
    {
        printf("Falha ao inicializar Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    cyw43_arch_gpio_put(LED_PIN, 0);
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        printf("Falha ao conectar ao Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    printf("Conectado ao Wi-Fi\n");

    if (netif_default)
    {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }
    // Configura o servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server)
    {
        printf("Falha ao criar servidor TCP\n");
    }

    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Falha ao associar servidor TCP à porta 80\n");
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    printf("Servidor ouvindo na porta 80\n");

    while (true)
    {
        cyw43_arch_poll();
    }

    cyw43_arch_deinit();
    return 0;
}
