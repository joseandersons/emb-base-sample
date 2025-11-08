<h1>Atividade 4 – Comunicação via ZBus e Sincronização com o SNTP</h1>

<h3> Implementação</h3>
<ul>
  <li><strong>Placa utilizada:</strong> ESP32-C3 SuperMini executando <strong>Zephyr RTOS</strong>.</li>
  <li><strong>Descrição:</strong><br>
    O sistema conecta-se a uma rede Wi-Fi, resolve o servidor SNTP e sincroniza o relógio interno do dispositivo.<br>
    O horário é publicado em um canal <strong>ZBus</strong> (<code>time_channel</code>), permitindo comunicação entre três threads:
    <ul>
      <li><strong>SNTP Thread:</strong> consulta o servidor NTP e publica o tempo atualizado.</li>
      <li><strong>Logger Thread:</strong> registra o timestamp recebido no console.</li>
      <li><strong>App Thread:</strong> calcula o intervalo (<code>Δt</code>) entre publicações sucessivas.</li>
    </ul>
  </li>
</ul>

<hr>

<h3>Estrutura do projeto</h3>
<ul>
  <li><code>src/wifi.c</code> — inicializa e conecta o Wi-Fi.</li>
  <li><code>src/sntp_task.c</code> — realiza sincronização SNTP assíncrona.</li>
  <li><code>src/logger_task.c</code> — imprime o horário atualizado.</li>
  <li><code>src/app_task.c</code> — calcula o tempo decorrido entre sincronizações.</li>
  <li><code>src/time_channel.c</code> — define o canal ZBus e estrutura de mensagem.</li>
  <li><code>src/main.c</code> — cria e gerencia as threads.</li>
  <li><code>prj.conf</code> — configura rede, Wi-Fi, DNS, SNTP e logs.</li>
  <li><code>Kconfig</code> — define parâmetros customizáveis (SSID, senha, fuso horário).</li>
</ul>

<hr>

<h3>Configuração</h3>
<p>Edite o arquivo <strong>prj.conf</strong> com suas credenciais:</p>
<pre>
CONFIG_WIFI_SSID="SeuSSID"
CONFIG_WIFI_PASSWD="SuaSenha"
CONFIG_SNTP_HOSTNAME="pool.ntp.org"
CONFIG_LOCAL_TIME=-3
</pre>

<hr>

<h3>Como compilar e gravar</h3>
<ol>
  <li>Conecte a placa <strong>ESP32-C3 SuperMini</strong> via USB.</li>
  <li>Compile e faça upload:
  <pre>
west build -b esp32c3_supermini . --pristine
west flash
west espressif monitor
  </pre></li>
</ol>

<hr>

<h3>Logs esperados</h3>
<pre>
[LOGGER] Sat 2025-11-08 00:12:03 UTC
[APP] dt=5s now=Sat 2025-11-08 00:12:08 UTC-3
</pre>

<hr>
