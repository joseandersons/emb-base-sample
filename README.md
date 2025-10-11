### Implementação

- **Plataforma:** ESP32‑C3 (SuperMini) com **LEDC** (controlador PWM da Espressif) via Zephyr.
- **Mapeamento (DeviceTree Overlay):**
  - **PWM**: LED em **GPIO7**, **canal LEDC 1**, exposto como `led0`.
  - **Botão**: **GPIO9** com `GPIO_PULL_UP | GPIO_ACTIVE_LOW`, exposto como `sw0`.
  - O canal LEDC é declarado como `channel@1 { reg = <1>; timer = <0>; }` e o pinmux como `LEDC_CH1_GPIO7`.
- **Aplicativo:**
  - **Modo Digital**: `k_timer` alterna 0%/100% de duty (pisca).
  - **Modo PWM (fade)**: variação de duty 0→100%→0 com passo configurável.
  - **Debounce por software** (≈150 ms) dentro do callback do botão.
  - Função auxiliar `pwm_set_or_log()` para checar erros de PWM.
- **Configuração por Kconfig (`CONFIG_APP_*`):**
  - `CONFIG_APP_PWM_PERIOD_NS` – período do PWM (ex.: `1000000` = 1 kHz).
  - `CONFIG_APP_BLINK_INTERVAL_MS` – intervalo do pisca digital.
  - `CONFIG_APP_FADE_STEP` – passo do fade em %.
  - `CONFIG_APP_FADE_DELAY_MS` – atraso entre passos do fade.
- **Logs**: `LOG_INF` para eventos de modo, `LOG_ERR` para falhas de PWM/dispositivos.

---

### Como compilar, gravar e monitorar

Comandos:
```bash
rm -rf build
west build -b esp32c3_supermini . --pristine
west flash
west espressif monitor
```

Saída esperada (exemplo):
```
*** Booting Zephyr OS build ...
<inf> app: Sistema iniciado! Pressione o botão (GPIO9) para alternar modos.
<inf> app: Modo digital (pisca) ativado
# Ao apertar o botão:
<inf> app: Modo PWM (fade) ativado
```

---

### Vídeo do funcionamento


- ▶️ **Assistir o vídeo:** [assets/atividade2.mp4](assets/atividade2.mp4)
