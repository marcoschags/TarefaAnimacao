#include <stdio.h>
#include "pico/stdlib.h"

// Bibliotecas para a matriz LED
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

// Frames para animação de coração pulsando
double coracao1[25] = {
    0.0, 0.3, 0.0, 0.3, 0.0,
    0.3, 0.3, 0.3, 0.3, 0.3,
    0.3, 0.3, 0.3, 0.3, 0.3,
    0.0, 0.3, 0.3, 0.3, 0.0,
    0.0, 0.0, 0.3, 0.0, 0.0
};

double coracao2[25] = {
    0.0, 0.0, 0.3, 0.0, 0.0,
    0.0, 0.3, 0.3, 0.3, 0.0,
    0.3, 0.3, 0.3, 0.3, 0.3,
    0.0, 0.3, 0.3, 0.3, 0.0,
    0.0, 0.0, 0.3, 0.0, 0.0
};

// Função para configurar o valor RGB
uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para desenhar na matriz
void desenho_pio(double *desenho, PIO pio, uint sm, double r, double g, double b) {
    uint32_t valor_led;
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(desenho[24 - i] * b, desenho[24 - i] * r, desenho[24 - i] * g);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para desligar todos os LEDs
void desligar_leds(PIO pio, uint sm) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, 0);
    }
}

// Função para ligar LEDs com uma cor uniforme
void ligar_leds_cor(PIO pio, uint sm, double r, double g, double b) {
    uint32_t valor_led = matrix_rgb(b, r, g);
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para animar o coração pulsando
void animar_coracao(PIO pio, uint sm) {
    for (int i = 0; i < 3; i++) { // Repetir animação 3 vezes
        desenho_pio(coracao1, pio, sm, 0.8, 0.0, 0.0); // Vermelho intenso
        sleep_ms(500);
        desenho_pio(coracao2, pio, sm, 0.8, 0.0, 0.0); // Vermelho fraco
        sleep_ms(500);
    }
}

int main() {
    // Inicializa comunicação serial
    stdio_init_all();

    // Inicializa o PIO
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &TarefaAnimacao_program);
    uint sm = pio_claim_unused_sm(pio, true);
    TarefaAnimacao_program_init(pio, sm, offset, OUT_PIN);

    // Configura colunas e linhas do teclado
    for (int i = 0; i < 4; i++) {
        gpio_init(teclas_colunas[i]);
        gpio_set_dir(teclas_colunas[i], GPIO_OUT);
        gpio_put(teclas_colunas[i], 1);
    }
    for (int i = 0; i < 4; i++) {
        gpio_init(teclas_linhas[i]);
        gpio_set_dir(teclas_linhas[i], GPIO_IN);
        gpio_pull_up(teclas_linhas[i]);
    }

    while (true) {
        char tecla = capturar_tecla();

        if (tecla != 'n') {
            printf("Tecla pressionada: %c\n", tecla);

            switch (tecla) {
                case '1':
                    animar_coracao(pio, sm);
                    break;
                case '3':
                    // Animação já implementada no código original
                    break;
                case 'A':
                    desligar_leds(pio, sm);
                    break;
                case 'B':
                    ligar_leds_cor(pio, sm, 0.0, 0.0, 1.0); // Azul, 100% intensidade
                    break;
                case 'C':
                    ligar_leds_cor(pio, sm, 0.8, 0.0, 0.0); // Vermelho, 80% intensidade
                    break;
                case 'D':
                    ligar_leds_cor(pio, sm, 0.0, 0.5, 0.0); // Verde, 50% intensidade
                    break;
                case '#':
                    ligar_leds_cor(pio, sm, 0.2, 0.2, 0.2); // Branco, 20% intensidade
                    break;
                default:
                    break;
            }
        }

        sleep_ms(200); // Pausa antes da próxima leitura
    }

    return 0;
}

// Função para capturar tecla pressionada
char capturar_tecla() {
    for (int i = 0; i < 4; i++) {
        gpio_put(teclas_colunas[i], 1);
    }

    for (int coluna = 0; coluna < 4; coluna++) {
        gpio_put(teclas_colunas[coluna], 0);

        for (int linha = 0; linha < 4; linha++) {
            if (gpio_get(teclas_linhas[linha]) == 0) {
                gpio_put(teclas_colunas[coluna], 1);
                return matriz_teclas[linha][coluna];
            }
        }

        gpio_put(teclas_colunas[coluna], 1);
    }

    return 'n'; // Nenhuma tecla pressionada
}
