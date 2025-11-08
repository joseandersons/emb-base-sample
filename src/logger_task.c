#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include "time_channel.h"
#include "logger_task.h"

LOG_MODULE_REGISTER(tracer, LOG_LEVEL_INF);

ZBUS_SUBSCRIBER_DEFINE(log_obs, 4);

void trace_worker(void *a, void *b, void *c)
{
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    const struct zbus_channel *ch;
    struct tmsg m;
    static struct tm clk = {0};
    char s[32];

    for (;;) {
        if (zbus_sub_wait(&log_obs, &ch, K_FOREVER) != 0) continue;
        if (zbus_chan_read(ch, &m, K_FOREVER) != 0) continue;

        clk = m.when;
        strftime(s, sizeof(s), "%a %Y-%m-%d %H:%M:%S %Z", &clk);
        LOG_INF("[LOG] %s", s);
    }
}
