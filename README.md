# U7T_projetofinal projeto final Embarcatech

Com o objetivo de consolidar os conceitos dados durante todo o curso sobre o uso de conversores analógico-digitais (ADC), PWM e comunicação I2C e demais funções na placa BitDogLAb como RP2040 da Raspberry Pi Puco, propõe-se a realização de uma atividade prática que simula o controle de umidade e a ativação de uma válvula solenoide (representada por um LED) com base em entradas de um joystick e botões.

## Objetivos
1) Compreender o funcionamento do conversor analógico-digital (ADC) no RP2040.
2) Utilizar o PWM para controlar a intensidade de um LED com base nos valores do joystick.
3) Exibir informações de umidade, potência do LED e umidade mínima no display SSD1306.
4) Aplicar o protocolo de comunicação I2C na integração com o display OLED.

## Descrição do Projeto
O joystick fornecerá valores analógicos correspondentes ao eixo Y, que serão utilizados para:

### Controlar a umidade simulada e a potência do LED:
1) O eixo Y do joystick ajusta a umidade simulada (0% a 100%) ou a potência do LED (0% a 100%), dependendo do modo de seleção.
2) O LED verde simula uma válvula solenoide, sendo acionado quando a umidade é menor que o limite mínimo configurado.
3) A potência do LED é controlada via PWM, permitindo variação suave da intensidade luminosa.

### Exibir informações no display SSD1306:
1) Umidade atual.
2) Potência do LED em porcentagem.
3) Umidade mínima configurada.
 
### Funcionalidades dos botões:
1) Botão do Joystick: Alterna entre o modo de ajuste de umidade e o modo de ajuste da potência do LED.
2) Botão B: Define o limite mínimo de umidade como o valor atual da umidade.

## Componentes Utilizados
- LED RGB (pino verde conectado à GPIO 11).
- Joystick (eixo Y conectado à GPIO 27).
- Botão do Joystick (conectado à GPIO 22).
- Botão B (conectado à GPIO 6).
- Display SSD1306 (conectado via I2C, GPIO 14 e GPIO 15).

## Funcionalidades Implementadas
- Função debounce:
Evita leituras múltiplas dos botões devido ao efeito de bounce.

- Função button_pressed_isr: 
Gerencia as interrupções dos botões sendo para o botão do joystick alterna o modo de seleção (umidade/potência do LED). E no caso do botão B ira definir a umidade mínima como o valor atual da umidade.

- Função setup_pwm:
Configura o PWM para controle do LED verde.

- Função pwm_to_percent:
Converte o valor PWM (0-4095) para porcentagem (0%-100%).

- Função principal (main):
Loop contínuo que gerencia a leitura do joystick, ajuste da umidade/potência do LED, controle do LED e atualização do display OLED.

## Vídeo Demonstrativo
- [Vídeo demonstrando o funcionamento](https://youtu.be/1LWWVySvQF4).

## Como Executar o Projeto
- Configuração do Ambiente:
Instale o VSCode e as extensões necessárias (C/C++, CMake Tools, Pico Debug).
Configure o ambiente com o SDK do Raspberry Pi Pico e as bibliotecas necessárias (ex.: hardware_adc, hardware_pwm, hardware_i2c).

- Compilação e Upload:
Use o CMake para configurar e compilar o projeto.
Carregue o arquivo .uf2 no Raspberry Pi Pico da placa BitDogLab.

- Execução:
O sistema começará a funcionar, començando com os valores de potencia e umidade inicial dimulada em 50% e a umidade minima em 25%. 
Para simular a variação de umidade basta movimentar o joystick para cima e para baixa aumentando ou diminuindo o valor da umidade, respectivamente.
Ao apertar o botão do jostick será alterada a função do movimento do joystick para aumentar ou diminuir a potencia do LED verde, que simula a abertura da valvula solenoide. Apertando novamente volta para os valores de umidade.
A qualquer momento poderá ser apertado o botão B, fazendo com que a umidade minima seja alterada para a umidade atual exibida.
Quando a umidade estiver abaixo da minima definida o LED verdade irá acender com a potencia que está definida, simulando a abertura do valvula solenoide para vazão da água, voltando a apagar quando o valor da umidade for igual ou maior ao minimo definido.