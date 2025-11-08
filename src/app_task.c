#include <inttypes.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/timeutil.h>
#include "time_channel.h"
#include "app_task.h"

LOG_MODULE_REGISTER(app_run, LOG_LEVEL_INF);

ZBUS_SUBSCRIBER_DEFINE(app_obs, 4);

void app_worker(void *a, void *b, void *c)
{
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    const struct zbus_channel *ch;
    struct tmsg m;
    static struct tm prev = {0};
    bool have_prev = false;

    char fmt[64], out[32];
    snprintf(fmt, sizeof(fmt), "%%a %%Y-%%m-%%d %%H:%%M:%%S %%Z%+d", CONFIG_LOCAL_TIME);

    for (;;) {
        if (zbus_sub_wait(&app_obs, &ch, K_FOREVER) != 0) continue;
        if (zbus_chan_read(ch, &m, K_FOREVER) != 0) continue;

        if (!have_prev) { prev = m.when; have_prev = true; }

        int64_t ta = timeutil_timegm64(&prev);
        int64_t tb = timeutil_timegm64(&m.when);
        int64_t dt = tb - ta;

        strftime(out, sizeof(out), fmt, &m.when);
        LOG_INF("[APP] dt=%" PRId64 "s now=%s", dt, out);

        prev = m.when;
    }
}
