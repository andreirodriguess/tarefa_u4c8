# tarefa_u4c8
# Projeto: Controle de Display OLED e LEDs com Joystick

## Informações do Aluno
- **Nome:** [Andrei Luiz da Silva Rodrigues]
- **Matrícula:** [tic370100444]
- **Vídeo de apresentação:** [https://youtu.be/pSBMZBntRjk?si=pQ6mj4L5K1d10VTx]

## Descrição do Projeto
Este projeto implementa um sistema interativo utilizando um joystick para controlar um display OLED SSD1306 via I2C, além de modificar a intensidade dos LEDs com PWM. O código é executado em uma Raspberry Pi Pico W, utilizando as bibliotecas adequadas para comunicação com o display e leitura dos sinais analógicos do joystick.

## Funcionalidades
- Controle de um quadrado no display OLED com base na posição do joystick.
- Animação de borda ao redor do display.
- Ajuste da intensidade dos LEDs vermelho e azul com base nos valores lidos do joystick.
- Detecção de pressionamento de botões para alternar entre animações e ligar/desligar os LEDs.

## Mapeamento de Pinos
- **VRX (Eixo X do Joystick):** GPIO 27
- **VRY (Eixo Y do Joystick):** GPIO 26
- **Botão do Joystick (PB):** GPIO 22
- **Botão A:** GPIO 5
- **LEDs RGB:** GPIO 11 (Verde), GPIO 12 (Azul), GPIO 13 (Vermelho)
- **I2C SDA:** GPIO 14
- **I2C SCL:** GPIO 15
