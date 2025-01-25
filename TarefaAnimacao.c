#include <stdio.h>
#include "pico/stdlib.h"

//BIBLIOTECAS PARA A MATRIZ LED
#include <math.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

// Arquivo .pio
#include "TarefaAnimacao.pio.h"

// Definições da matriz de LEDs
#define NUM_PIXELS 25
#define OUT_PIN 9
// Pino do buzzer
#define BUZZ 12


// Pinos para o teclado matricial
const uint8_t teclas_colunas[4] = {4, 3, 2, 1};
const uint8_t teclas_linhas[4] = {8, 7, 6, 5};


// Mapeamento do teclado matricial
const char matriz_teclas[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

char capturar_tecla();

// Vetores de imagens para a matriz de LEDs
double desenho1[25] = {
    0.0, 0.3, 0.0, 0.3, 0.0,
    0.0, 0.3, 0.3, 0.0, 0.3,
    0.3, 0.0, 0.3, 0.0, 0.0,
    0.3, 0.0, 0.0, 0.3, 0.0,
    0.0, 0.3, 0.3, 0.0, 0.3
    };

double desenho2[25] = {
    0.3, 0.0, 0.3, 0.0, 0.0,
    0.0, 0.3, 0.3, 0.3, 0.0,
    0.3, 0.0, 0.0, 0.3, 0.0,
    0.0, 0.0, 0.3, 0.0, 0.3,
    0.0, 0.3, 0.0, 0.3, 0.3
};

double desenho3[25] = {
    0.3, 0.0, 0.0, 0.3, 0.0,
    0.3, 0.0, 0.3, 0.0, 0.0,
    0.0, 0.3, 0.3, 0.3, 0.0,
    0.3, 0.3, 0.0, 0.3, 0.0,
    0.0, 0.0, 0.3, 0.0, 0.3
};

double desenho4[25] = {
    0.0, 0.0, 0.3, 0.0, 0.3,
    0.3, 0.3, 0.0, 0.3, 0.0,
    0.0, 0.0, 0.3, 0.3, 0.0,
    0.3, 0.3, 0.0, 0.0, 0.3,
    0.0, 0.3, 0.0, 0.3, 0.0
};

double desenho5[25] = {
    0.0, 0.3, 0.0, 0.0, 0.3,
    0.3, 0.0, 0.0, 0.3, 0.3,
    0.0, 0.3, 0.0, 0.3, 0.0,
    0.3, 0.0, 0.3, 0.0, 0.0,
    0.0, 0.3, 0.3, 0.0, 0.3
};

// Desenhos para a animação da tecla '3' 
double frame1[25] = {
    0.2, 0.0, 0.2, 0.0, 0.2,
    0.0, 0.2, 0.0, 0.2, 0.0,
    0.2, 0.0, 0.2, 0.0, 0.2,
    0.0, 0.2, 0.0, 0.2, 0.0,
    0.2, 0.0, 0.2, 0.0, 0.2
};

double frame2[25] = {
    0.0, 0.2, 0.0, 0.2, 0.0,
    0.2, 0.0, 0.2, 0.0, 0.2,
    0.0, 0.2, 0.0, 0.2, 0.0,
    0.2, 0.0, 0.2, 0.0, 0.2,
    0.0, 0.2, 0.0, 0.2, 0.0
};

double frame3[25] = {
    0.2, 0.2, 0.2, 0.2, 0.2,
    0.2, 0.0, 0.0, 0.0, 0.2,
    0.2, 0.0, 0.2, 0.0, 0.2,
    0.2, 0.0, 0.0, 0.0, 0.2,
    0.2, 0.2, 0.2, 0.2, 0.2
};

double frame4[25] = {
    0.0, 0.0, 0.2, 0.0, 0.0,
    0.0, 0.2, 0.0, 0.2, 0.0,
    0.2, 0.0, 0.0, 0.0, 0.2,
    0.0, 0.2, 0.0, 0.2, 0.0,
    0.0, 0.0, 0.2, 0.0, 0.0
};

double frame5[25] = {
    0.2, 0.2, 0.0, 0.2, 0.2,
    0.2, 0.0, 0.2, 0.0, 0.2,
    0.0, 0.2, 0.0, 0.2, 0.0,
    0.2, 0.0, 0.2, 0.0, 0.2,
    0.2, 0.2, 0.0, 0.2, 0.2
};

// Função para imprimir valor binário
void imprimir_binario(int num) {
    int i;
    for (i = 31; i >= 0; i--) {
        (num & (1 << i)) ? printf("1") : printf("0");
    }
}

// Função para configurar a matriz de LEDs
uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para acionar a matriz de LEDs
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        if (i % 2 == 0) {
            valor_led = matrix_rgb(desenho[24 - i], r = 0.0, g = 0.0);
            pio_sm_put_blocking(pio, sm, valor_led);
        } else {
            valor_led = matrix_rgb(b = 0.0, desenho[24 - i], g = 0.0);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
    }
    imprimir_binario(valor_led);
}

// Função para ligar todos os LEDs no vermelho com 80% de intensidade
void ligar_leds_vermelho(PIO pio, uint sm) {
    uint32_t valor_led = matrix_rgb(0.0, 0.8, 0.0); // 80% de intensidade no vermelho
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para ligar todos os LEDs na cor verde com 50% de intensidade
void ligar_leds_verde(PIO pio, uint sm) {
    uint32_t valor_led = matrix_rgb(0.0, 0.0, 0.5); // 50% de intensidade no verde
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

int main()
{

    
    // Inicializa comunicação serial
    stdio_init_all();

PIO pio = pio0;
    bool ok;
    uint16_t i;
    uint32_t valor_led;
    double r = 0.0, b = 0.0, g = 0.0;

    // Configura o clock para 128 MHz
    ok = set_sys_clock_khz(128000, false);
    printf("Iniciando a transmissão PIO\n");

    // Configurações do PIO
    uint offset = pio_add_program(pio, &TarefaAnimacao_program);
    uint sm = pio_claim_unused_sm(pio, true);
    TarefaAnimacao_program_init(pio, sm, offset, OUT_PIN);    

    // Configura buzzer como saída
    gpio_init(BUZZ);
    gpio_set_dir(BUZZ, GPIO_OUT);
    gpio_put(BUZZ, false);

    // Configura colunas como saídas digitais
    for (int i = 0; i < 4; i++) 
    {
        gpio_init(teclas_colunas[i]);
        gpio_set_dir(teclas_colunas[i], GPIO_OUT);
        gpio_put(teclas_colunas[i], 1); // Colunas iniciam em alto
    }

    // Configura linhas como entradas com pull-up
    for (int i = 0; i < 4; i++) 
    {
        gpio_init(teclas_linhas[i]);
        gpio_set_dir(teclas_linhas[i], GPIO_IN);
        gpio_pull_up(teclas_linhas[i]); // Habilita pull-up nas linhas
    }

    

    while (true) 
    {
        char tecla = capturar_tecla();

        if (tecla != 'n') // Só executa se uma tecla for pressionada
        {
            printf("Tecla pressionada: %c\n", tecla);

            // Ações para cada tecla
            switch (tecla) 
            {
                case '3':
                    desenho_pio(frame1, valor_led, pio, sm, r, g, b); // Frame 1
                    sleep_ms(500);
                    desenho_pio(frame2, valor_led, pio, sm, r, g, b); // Frame 2
                    sleep_ms(500);
                    desenho_pio(frame3, valor_led, pio, sm, r, g, b); // Frame 3
                    sleep_ms(500);
                    desenho_pio(frame4, valor_led, pio, sm, r, g, b); // Frame 4
                    sleep_ms(500);
                    desenho_pio(frame5, valor_led, pio, sm, r, g, b); // Frame 5
                    sleep_ms(500);
                    break;
                case '6':
                    desenho_pio(desenho1, valor_led, pio, sm, r, g, b); // Ação para o padrão 1
                    sleep_ms(500);
                    desenho_pio(desenho2, valor_led, pio, sm, r, g, b); // Ação para o padrão 2 
                    sleep_ms(500);                    
                    desenho_pio(desenho3, valor_led, pio, sm, r, g, b); // Ação para o padrão 3                   
                    sleep_ms(500);
                    desenho_pio(desenho4, valor_led, pio, sm, r, g, b); // Ação para o padrão 4                   
                    sleep_ms(500);
                    desenho_pio(desenho5, valor_led, pio, sm, r, g, b); // Ação para o padrão                    
                    sleep_ms(500);
                    break;

                case 'C':
                    ligar_leds_vermelho(pio, sm); // Liga todos os LEDs no vermelho com 80% de intensidade
                    sleep_ms(500);
                    break;
                case 'D':
                    ligar_leds_verde(pio, sm); // Liga todos os LEDs na cor verde com 50% de intensidade
                    sleep_ms(500);
                    break;
                default:
                    break;
            }
        }

        sleep_ms(200); // Pausa antes da próxima leitura
    }

    return 0;
}

// Função para identificar a tecla pressionada
char capturar_tecla() {
    // Reseta todas as colunas
    for (int i = 0; i < 4; i++) {
        gpio_put(teclas_colunas[i], 1);
    }

    for (int coluna = 0; coluna < 4; coluna++) {
        gpio_put(teclas_colunas[coluna], 0); // Ativa a coluna atual

        for (int linha = 0; linha < 4; linha++) {
            if (gpio_get(teclas_linhas[linha]) == 0) { // Detecta pressão na linha
                gpio_put(teclas_colunas[coluna], 1); // Desativa a coluna atual
                return matriz_teclas[linha][coluna]; // Retorna imediatamente
            }
        }

        gpio_put(teclas_colunas[coluna], 1); // Desativa a coluna atual
    }

    return 'n'; // Nenhuma tecla pressionada
}