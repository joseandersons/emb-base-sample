#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app);

#define INTERVAL_MS CONFIG_APP_TIMER_INTERVAL_MSS

static struct k_timer hello_timer;

void hello_timer_handler(struct k_timer *timer_id)
{
    LOG_INF("Hello World periódico! Intervalo = %d ms", INTERVAL_MS);
    LOG_DBG("Mensagem em nível DEBUG");
    LOG_ERR("Mensagem em nível ERROR");
}

int main(void)
{

    k_timer_init(&hello_timer, hello_timer_handler, NULL);
    k_timer_start(&hello_timer, K_MSEC(INTERVAL_MS), K_MSEC(INTERVAL_MS));

    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
