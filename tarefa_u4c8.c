#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "pico/bootrom.h"

// Definição dos pinos dos LEDs
#define GREEN_PIN 11
#define BLUE_PIN 12
#define RED_PIN 13

// Definição dos pinos do joystick e do botão
#define VRX_PIN 27
#define VRY_PIN 26
#define PB 22          // Pino para o botão do joystick
#define button_a 5     // Pino para o botão A

// Definição dos pinos do display I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C  // Endereço do display OLED

// Outras bibliotecas necessárias para o funcionamento do display e ADC
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/adc.h"

// Função para inicializar o PWM em um pino GPIO
uint pwm_init_gpio(uint gpio, uint wrap)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);  // Configura o pino GPIO para função PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio);  // Obtem o número do slice PWM associado ao pino
    pwm_set_wrap(slice_num, wrap);  // Define o valor de "wrap", que controla a resolução do PWM
    pwm_set_enabled(slice_num, true);  // Habilita o PWM no slice
    return slice_num;  // Retorna o número do slice PWM
}

// Função de interrupção do GPIO (botões)
void gpio_irq_handler(uint gpio, uint32_t events);
uint32_t last_print_time = 0;  // Variável para controle de tempo de impressão (1 segundo)
uint32_t last_joystick_button_time = 0;  // Controle de debounce do botão do joystick
uint32_t last_button_a_time = 0;  // Controle de debounce do botão A
bool estado_leds = true;  // Controle do estado dos LEDs (ligado/desligado)

int main()
{
    stdio_init_all();  // Inicializa a comunicação serial (para debug)

    adc_init();  // Inicializa o ADC
    adc_gpio_init(VRX_PIN);  // Inicializa o pino do eixo X do joystick
    adc_gpio_init(VRY_PIN);  // Inicializa o pino do eixo Y do joystick

    uint pwm_wrap = 4096;  // Valor de wrap do PWM (resolução de 12 bits)
    uint pwm_slice_red = pwm_init_gpio(RED_PIN, pwm_wrap);  // Inicializa o PWM do LED vermelho
    uint pwm_slice_blue = pwm_init_gpio(BLUE_PIN, pwm_wrap);  // Inicializa o PWM do LED azul

    // Configuração dos pinos GPIO para LEDs e botões
    gpio_init(GREEN_PIN);  // Inicializa o pino do LED verde
    gpio_set_dir(GREEN_PIN, GPIO_OUT);  // Define o pino do LED verde como saída
    gpio_init(PB);  // Inicializa o pino do botão do joystick
    gpio_set_dir(PB, GPIO_IN);  // Define o pino do botão como entrada
    gpio_pull_up(PB);  // Habilita o resistor pull-up no botão
    gpio_init(button_a);  // Inicializa o pino do botão A
    gpio_set_dir(button_a, GPIO_IN);  // Define o pino do botão A como entrada
    gpio_pull_up(button_a);  // Habilita o resistor pull-up no botão A

    // Habilita interrupções para os botões com função de callback
    gpio_set_irq_enabled_with_callback(PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_a, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicialização do I2C (usado para comunicação com o display)
    i2c_init(I2C_PORT, 400 * 1000);  // Define a velocidade do I2C para 400 KHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Configura o pino SDA para função I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // Configura o pino SCL para função I2C
    gpio_pull_up(I2C_SDA);  // Habilita pull-up no pino SDA
    gpio_pull_up(I2C_SCL);  // Habilita pull-up no pino SCL

    // Inicializa o display SSD1306
    ssd1306_t ssd;  // Cria uma estrutura para o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);  // Inicializa o display com o endereço I2C
    ssd1306_config(&ssd);  // Configura o display (opções como inversão de cores)
    ssd1306_send_data(&ssd);  // Envia as configurações para o display

    // Limpa o display (apaga todos os pixels)
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    while (true)
    {
        // Funções ADC: leitura dos valores do joystick
        adc_select_input(0);  // Seleciona o canal do eixo y
        uint16_t vry_value = adc_read();  // Lê o valor do eixo y
        
        adc_select_input(1);  // Seleciona o canal do eixo x
        uint16_t vrx_value = adc_read();  // Lê o valor do eixo x
        
        // Converte os valores de ADC para duty cycle do PWM (intensidade dos LEDs)
        float duty_cycle_red = get_duty_cycle(vrx_value);  // Cálculo para o LED vermelho
        float duty_cycle_blue = get_duty_cycle(vry_value);  // Cálculo para o LED azul

        // Mapeamento dos valores do joystick para as coordenadas do display
        uint8_t x = map_x_to_display(vrx_value);  // Converte o valor do eixo X para coordenada X do display
        uint8_t y = map_y_to_display(vry_value);  // Converte o valor do eixo Y para coordenada Y do display

        // Atualiza os LEDs de acordo com o joystick, se os LEDs estiverem ativados
        if (estado_leds == true)
        {
            pwm_set_gpio_level(RED_PIN, duty_cycle_red);  // Atualiza o PWM do LED vermelho
            pwm_set_gpio_level(BLUE_PIN, duty_cycle_blue);  // Atualiza o PWM do LED azul
        }

        uint32_t current_time = to_ms_since_boot(get_absolute_time());  // Obtém o tempo atual

        // Imprime os valores no terminal a cada 1 segundo
        if (current_time - last_print_time >= 1000)
        {
            printf("VRX: %u\n", vrx_value);  // Valor do eixo X
            printf("Duty Cycle RED LED : %.2f%%\n", duty_cycle_red / 4095 * 100);  // Duty cycle do LED vermelho
            printf("x = %d, y = %d\n", x, y);  // Coordenadas X e Y no display
            printf("VRY: %u\n", vry_value);  // Valor do eixo Y
            printf("Duty Cycle BLUE LED: %.2f%%\n", duty_cycle_blue / 4095 * 100);  // Duty cycle do LED azul
            last_print_time = current_time;  // Atualiza o tempo de impressão
        }

        // Atualiza o conteúdo do display com animações
        bool cor = true;  // Define a cor da animação
        ssd1306_fill(&ssd, !cor);  // Limpa o display

        // Se o botão do joystick for pressionado, desenha uma borda retangular
        if (gpio_get(PB))
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);  // Desenha a borda
        else
            ssd1306_animated_border(&ssd, cor);  // Desenha a borda animada

        // Desenha um quadrado nas coordenadas (x, y) calculadas do joystick
        ssd1306_draw_square(&ssd, x, y, cor);  // Desenha o quadrado
        ssd1306_send_data(&ssd);  // Atualiza o display
        sleep_ms(40);  // Aguarda 40 ms para criar o efeito de animação
    }
}

// Função de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time());  // Obtém o tempo atual

    if (gpio == PB)  // Se o botão do joystick foi pressionado
    {
        if (current_time - last_joystick_button_time >= 200)  // Debounce (evita múltiplas leituras)
        {
            gpio_put(GREEN_PIN, !gpio_get(GREEN_PIN));  // Alterna o estado do LED verde
            last_joystick_button_time = current_time;  // Atualiza o tempo de debounce
        }
    }
    else if (gpio == button_a)  // Se o botão A foi pressionado
    {
        if (current_time - last_button_a_time >= 200)  // Debounce
        {
            estado_leds = !estado_leds;  // Alterna o estado dos LEDs (liga/desliga)
            last_button_a_time = current_time;  // Atualiza o tempo de debounce
        }
    }
}
