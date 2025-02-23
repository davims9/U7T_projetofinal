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
volatile uint32_t last_button_joystick_time = 0;
volatile uint32_t last_button_b_time = 0;
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
    
    while (1) {
        adc_select_input(0);
        uint16_t x_valor = adc_read();
        
        if (modo_selecao) {
            if (x_valor < 4000 && potencia_led > 100) potencia_led -= 100;
            if (x_valor > 50 && potencia_led < 4095) potencia_led += 100;
        } else {
            if (x_valor < 4000 && humidade > 0) humidade--;
            if (x_valor > 50 && humidade < 100) humidade++;
        }
        
        pwm_set_gpio_level(LED_GREEN, (humidade < humidade_minima) ? potencia_led : 0);
        
        ssd1306_fill(&ssd, false);
        char buffer[50];
        sprintf(buffer, "UMIDADE: %d%%", humidade);
        ssd1306_draw_string(&ssd, buffer, 0, 0);
        sprintf(buffer, "LED: %d", potencia_led);
        ssd1306_draw_string(&ssd, buffer, 0, 20);
        sprintf(buffer, "UMIDADE MIN: %d%%", humidade_minima);
        ssd1306_draw_string(&ssd, buffer, 0, 40);
        ssd1306_send_data(&ssd);
        
        sleep_ms(100);
    }
    return 0;
}
