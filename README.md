#  Comunicação entre Threads – Produtores, Filtro e Consumidor

### Implementação

- **Placa utilizada:** ESP32-C3 SuperMini executando **Zephyr RTOS**.  
- **Descrição:**  
  O sistema simula dois sensores (temperatura e umidade) que enviam dados para uma fila de entrada.  
  Uma thread intermediária (**filtro**) valida os valores e envia apenas os corretos para uma fila de saída,  
  que é lida por uma thread **consumidora**, responsável por exibir os resultados no console.

---

### Como testar

1. Conecte a placa **ESP32-C3 SuperMini** via USB.
2. Compile e grave o firmware:
   ```bash
   rm -rf build
   west build -b esp32c3_supermini . --pristine
   west flash
   west espressif monitor
