#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"

// Definições dos pinos
#define LED_GREEN 11
#define JOYSTICK_BUTTON 22
#define JOYSTICK_Y_PIN 27
#define BUTTON_B_PIN 6
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define I2C_PORT i2c1
#define ENDERECO 0x3C

// Variáveis globais
volatile bool modo_selecao = false;
volatile uint16_t potencia_led = 2048; // 50% de 4095
volatile uint16_t humidade = 50; // 50%
volatile uint16_t humidade_minima = 25; // Limite de umidade inicial
volatile uint32_t last_button_joystick_time = 200;
volatile uint32_t last_button_b_time = 200;
ssd1306_t ssd;

// Função de debouncing
bool debounce(uint32_t *last_time) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - *last_time > 200) {
        *last_time = current_time;
        return true;
    }
    return false;
}

// Interrupção do botão do joystick
void button_pressed_isr(uint gpio, uint32_t events) {
    if (gpio == JOYSTICK_BUTTON && debounce(&last_button_joystick_time)) {
        modo_selecao = !modo_selecao;
    } else if (gpio == BUTTON_B_PIN && debounce(&last_button_b_time)) {
        humidade_minima = humidade; // Atualiza o limite de umidade para o valor atual
    }
}

// Configuração do PWM
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 4095);
    pwm_set_enabled(slice_num, true);
}

// Função para converter valor de 0-4095 para porcentagem (0%-100%)
uint8_t pwm_to_percent(uint16_t pwm_value) {
    return (uint8_t)((pwm_value * 100) / 4095);
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(JOYSTICK_Y_PIN);
    setup_pwm(LED_GREEN);
    
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_pressed_isr);
    
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &button_pressed_isr);
    
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    ssd1306_init(&ssd, 128, 64, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd); // Configura o ssd
    ssd1306_send_data(&ssd); // Envia os dados para o ssd

    // Limpa o ssd. O ssd inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    
    while (1) {
        adc_select_input(0);
        uint16_t y_valor = adc_read();
        
        if (modo_selecao) {
            if (y_valor < 4000 && potencia_led > 50) potencia_led -= 50;
            if (y_valor > 50 && potencia_led < 4095) potencia_led += 50;
        } else {
            if (y_valor < 4000 && humidade > 0) humidade--;
            if (y_valor > 50 && humidade < 100) humidade++;
        }
        
        pwm_set_gpio_level(LED_GREEN, (humidade < humidade_minima) ? potencia_led : 0);
        
        ssd1306_fill(&ssd, false);
        char buffer[50];
        sprintf(buffer, "UMIDADE %d%%", humidade);
        ssd1306_draw_string(&ssd, buffer, 0, 0);

        // Exibe a potência do LED em porcentagem
        uint8_t potencia_percent = pwm_to_percent(potencia_led);
        sprintf(buffer, "LED %d%% ", potencia_percent);
        ssd1306_draw_string(&ssd, buffer, 0, 20);

        sprintf(buffer, "UMIDADE MIN %d%%", humidade_minima);
        ssd1306_draw_string(&ssd, buffer, 0, 40);
        ssd1306_send_data(&ssd);
        
        sleep_ms(100);
    }
    return 0;
}