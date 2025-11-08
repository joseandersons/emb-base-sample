#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket_service.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/timeutil.h>
#include "time_channel.h"
#include "sntp_task.h"

LOG_MODULE_REGISTER(ntp, LOG_LEVEL_INF);

struct ep_s {
    struct sockaddr addr;
    socklen_t len;
};

static struct sntp_time st;
static K_SEM_DEFINE(sig_ntp, 0, 1);

static void ntp_svc(struct net_socket_service_event *ev)
{
    if (sntp_read_async(ev, &st) == 0) {
        k_sem_give(&sig_ntp);
    }
}

NET_SOCKET_SERVICE_SYNC_DEFINE_STATIC(svc_ntp, ntp_svc, 1);

void ntp_worker(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct ep_s *ep = (struct ep_s *)arg1;
    struct sntp_ctx ctx;
    if (sntp_init_async(&ctx, &ep->addr, ep->len, &svc_ntp) != 0) {
        sntp_close(&ctx);
        return;
    }

    char fmt[64];
    snprintf(fmt, sizeof(fmt), "%%a %%Y-%%m-%%d %%H:%%M:%%S %%Z%+d", CONFIG_LOCAL_TIME);

    for (;;) {
        k_sem_reset(&sig_ntp);
        if (sntp_send_async(&ctx) != 0) {
            k_sleep(K_SECONDS(1));
            continue;
        }
        if (k_sem_take(&sig_ntp, K_SECONDS(1)) != 0) {
            k_sleep(K_MSEC(500));
            continue;
        }

        const struct timespec ts = {
            .tv_sec = st.seconds,
            .tv_nsec = (long)((((uint64_t)st.fraction) * 1000000000ULL) >> 32)
        };
        sys_clock_settime(CLOCK_REALTIME, &ts);

        uint64_t local_sec = st.seconds + (CONFIG_LOCAL_TIME * 3600);
        struct tm tloc;
        gmtime_r(&local_sec, &tloc);

        struct tmsg msg = { .when = tloc };
        zbus_chan_pub(&time_channel, &msg, K_NO_WAIT);

        int delay_ms = (k_cycle_get_32() % 5000) + 500;
        k_sleep(K_MSEC(delay_ms));
    }

    sntp_close_async(&svc_ntp);
    sntp_close(&ctx);
}
