#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <stdbool.h>
#include <zephyr/random/random.h>

/* Inicializa módulo de logging */
LOG_MODULE_REGISTER(app);

/* Configurações via Kconfig */
#define Q_IN_LEN                CONFIG_APP_Q_IN_LEN
#define Q_OUT_LEN               CONFIG_APP_Q_OUT_LEN
#define PROD_TEMP_PERIOD_MS     CONFIG_APP_PROD_TEMP_PERIOD_MS
#define PROD_UMID_PERIOD_MS     CONFIG_APP_PROD_UMID_PERIOD_MS
#define TEMP_MIN_C              CONFIG_APP_TEMP_MIN_C
#define TEMP_MAX_C              CONFIG_APP_TEMP_MAX_C
#define UMID_MIN_PCT            CONFIG_APP_UMID_MIN_PCT
#define UMID_MAX_PCT            CONFIG_APP_UMID_MAX_PCT

/* Parâmetros das threads */
#define STACK_SIZE 1024
#define PRIO_PROD 4
#define PRIO_FILTER 3
#define PRIO_CONS 5

typedef enum {
    SENSOR_TEMP = 0,
    SENSOR_UMID = 1
} sensor_type_t;

typedef struct {
    sensor_type_t type;  
    float         value; 
    int64_t       ts_ms; 
} sensor_msg_t;

/* Inicializa filas de mensagens de sensores */
K_MSGQ_DEFINE(q_in,  sizeof(sensor_msg_t), Q_IN_LEN,  8);
K_MSGQ_DEFINE(q_out, sizeof(sensor_msg_t), Q_OUT_LEN, 8);


/* -------------------- Mock dos Sensores -------------------- */
/* Gera inteiros uniformes de 0 a 100  */
static inline float mock_0_100(void) {
    return (float)(sys_rand32_get() % 101u);
}

static float mock_temp_c(void) {
    return mock_0_100();
}

static float mock_umid_pct(void) {
    return mock_0_100();
}

/* ---- Validação isolada ---- */
static inline bool is_temp_ok(float v) {
    return v >= (float)TEMP_MIN_C && v <= (float)TEMP_MAX_C;
}
static inline bool is_umid_ok(float v) {
    return v >= (float)UMID_MIN_PCT && v <= (float)UMID_MAX_PCT;
}
static inline bool validate(const sensor_msg_t *m) {
    return (m->type == SENSOR_TEMP) ? is_temp_ok(m->value) : is_umid_ok(m->value);
}

/* Produtor: Temperatura */
static void producer_temp(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    k_thread_name_set(k_current_get(), "prod_temp");

    while (1) {
        sensor_msg_t m = {
            .type  = SENSOR_TEMP,
            .value = mock_temp_c(),
            .ts_ms = k_uptime_get()
        };

        (void)k_msgq_put(&q_in, &m, K_FOREVER);
        k_msleep(PROD_TEMP_PERIOD_MS);
    }
}

/* Produtor: Umidade */
static void producer_umid(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    k_thread_name_set(k_current_get(), "prod_umid");

    while (1) {
        sensor_msg_t m = {
            .type  = SENSOR_UMID,
            .value = mock_umid_pct(), 
            .ts_ms = k_uptime_get()
        };

        (void)k_msgq_put(&q_in, &m, K_FOREVER);
        k_msleep(PROD_UMID_PERIOD_MS);
    }
}

/* Filtro */
static void filter_thread(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    k_thread_name_set(k_current_get(), "filter");
    sensor_msg_t m;

    while (1) {
        k_msgq_get(&q_in, &m, K_FOREVER);

        if (validate(&m)) {
            (void)k_msgq_put(&q_out, &m, K_FOREVER);

        } else {
            if (m.type == SENSOR_TEMP) {
                LOG_WRN("TEMP fora: %.2f C @ %lld ms (aceito: %d..%d)",
                        (double)m.value, (long long)m.ts_ms, TEMP_MIN_C, TEMP_MAX_C);
            } else {
                LOG_WRN("UMID fora: %.2f %% @ %lld ms (aceito: %d..%d)",
                        (double)m.value, (long long)m.ts_ms, UMID_MIN_PCT, UMID_MAX_PCT);
            }
        }
    }
}

/* ---- Consumidor: lê apenas q_out ---- */
static void consumer_thread(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    k_thread_name_set(k_current_get(), "consumer");
    sensor_msg_t m;

    while (1) {
        k_msgq_get(&q_out, &m, K_FOREVER);

        if (m.type == SENSOR_TEMP) {
            LOG_INF("OK -> Temp: %.2f C (t=%lld ms)", (double)m.value, (long long)m.ts_ms);
        } else {
            LOG_INF("OK -> Umid: %.2f %% (t=%lld ms)", (double)m.value, (long long)m.ts_ms);
        }
    }
}

/* Criação estática das threads produtoras */
K_THREAD_DEFINE(t_prod_temp, STACK_SIZE, producer_temp, NULL, NULL, NULL, PRIO_PROD, 0, 0);
K_THREAD_DEFINE(t_prod_umid, STACK_SIZE, producer_umid, NULL, NULL, NULL, PRIO_PROD, 0, 50);                                                                                                                
K_THREAD_DEFINE(t_filter, STACK_SIZE, filter_thread,  NULL, NULL, NULL, PRIO_FILTER, 0, 0);
K_THREAD_DEFINE(t_cons,   STACK_SIZE, consumer_thread,NULL, NULL, NULL, PRIO_CONS,   0, 0);                                                                                                                                                                                                                                                                                                                                             

int main(void) {
    LOG_INF("Pipeline Produtores -> Filtro -> Consumidor iniciado.");
    return 0;
}

