# Radar de Veículos com Zephyr RTOS

Sistema de radar/simulador para detecção de veículos utilizando **Zephyr RTOS**, com:
- Contagem de eixos por sensores;
- Cálculo de velocidade;
- Classificação de veículo (leve/pesado);
- Validação de infração com limites configuráveis;
- Integração com serviço de câmera via **zbus**;
- Validação de placa (Brasil e países do Mercosul);
- Saída em console com cores ANSI.

---

## Visão Geral

O projeto simula um radar de fiscalização:

1. Dois sensores (GPIO) detectam a passagem do veículo.
2. O tempo entre os sensores é usado para calcular a velocidade.
3. O número de ativações do sensor 0 é usado para inferir o tipo de veículo:
   - 2 passagens → veículo leve
   - \> 2 passagens → veículo pesado
4. O módulo `radar_core` compara a velocidade com o limite configurado.
5. Em caso de infração:
   - Dispara uma captura de imagem (`camera_service`).
   - Recebe os dados da câmera via **zbus**.
   - Valida a placa (`plate_validator`).
6. O módulo de display imprime o resultado no console com cores ANSI.

---

## Principais Módulos

### `main.c`

Responsável por:

- Inicializar:
  - GPIO dos sensores (`button0` e `button1` via Devicetree);
  - Callbacks de interrupção;
  - Filas de mensagens (`display_queue`, `sensor_queue`, `vehicle_queue`);
  - Threads (`control_thread`, `sensor_thread`, `display_thread`);
  - Inscrição no canal zbus da câmera.

- Configurar interrupções:
  - `GPIO_INT_EDGE_TO_ACTIVE` em `sensor0` e `sensor1`.

- Loop principal:
  - Lê caracteres da UART (`console_dev`) e, para teste/emulação:
    - Tecla `'1'` → aciona `sensor0` via `gpio_emul_input_set`.
    - Tecla `'2'` → aciona `sensor1` via `gpio_emul_input_set`.

### `sensor_thread.c`

Responsável por transformar eventos dos sensores em dados de veículo.

- Usa o `enum STATE`:
  - `IDLE` → aguardando primeiro sensor
  - `READING` → veículo em leitura
  - `DONE` → finaliza o processamento do veículo

- Estruturas:
  - `struct data` com:
    - `item.from` / `item.type`
    - `value.timestamp` / `value.speed`
    - `eixos` (contagem de ativações do sensor 0)

- Lógica básica:
  - `IDLE`:
    - Ao receber `SENSOR_0`:
      - Salva `init_timestamp`
      - Zera contadores
      - Inicia `timeout_vehicle` (700 ms)
      - Vai para `READING`
  - `READING`:
    - Novo `SENSOR_0`: incrementa `count_sensor_0`, reinicia o timer.
    - `SENSOR_1`:
      - Calcula `dt = timestamp_sensor1 - init_timestamp`.
      - Calcula velocidade:
        ```c
        vehicle.value.speed = (CONFIG_RADAR_SENSOR_DISTANCE_MM / dt) * 3.6;
        ```
    - `TIMER`: muda para `DONE`.
  - `DONE`:
    - Se não houver velocidade calculada → descarta.
    - Classifica tipo:
      - `count_sensor_0 == 2` → `LEVE`
      - `count_sensor_0 > 2` → `PESADO`
    - Define `vehicle.eixos = count_sensor_0`.
    - Envia `vehicle` para `vehicle_queue`.
    - Volta para `IDLE`.

### `control_thread.c`

Responsável pela lógica de negócio do radar.

- Lê `struct data vehicle` da `vehicle_queue`.
- Converte `vehicle.item.type` em `enum tipo_veiculo_t`:
  - `LEVE` → `VEICULO_LEVE`
  - `PESADO` → `VEICULO_PESADO`
- Chama:
  ```c
  struct radar_result_t r = radar_analisar((int)vehicle.value.speed, tipo);
