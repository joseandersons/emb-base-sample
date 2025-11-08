#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>
#include "wifi.h"
#include "sntp_task.h"
#include "logger_task.h"
#include "app_task.h"

LOG_MODULE_REGISTER(entry, LOG_LEVEL_INF);

K_THREAD_STACK_DEFINE(stk_ntp, 1024);
K_THREAD_STACK_DEFINE(stk_log, 1024);
K_THREAD_STACK_DEFINE(stk_app, 1024);

static struct k_thread th_ntp;
static struct k_thread th_log;
static struct k_thread th_app;

struct ep_s {
    struct sockaddr addr;
    socklen_t len;
};

static struct ep_s ntp_ep;

int main(void)
{
    if (!net_up) {
        return -1;
    }

    struct zsock_addrinfo hints = {0}, *res = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (zsock_getaddrinfo(CONFIG_SNTP_HOSTNAME, "123", &hints, &res) != 0) {
        return -1;
    }

    ntp_ep.len = res->ai_addrlen;
    memcpy(&ntp_ep.addr, res->ai_addr, res->ai_addrlen);
    zsock_freeaddrinfo(res);

    k_thread_create(&th_ntp, stk_ntp, K_THREAD_STACK_SIZEOF(stk_ntp),
                    ntp_worker, &ntp_ep, NULL, NULL, K_PRIO_PREEMPT(4), 0, K_NO_WAIT);

    k_thread_create(&th_log, stk_log, K_THREAD_STACK_SIZEOF(stk_log),
                    trace_worker, NULL, NULL, NULL, K_PRIO_PREEMPT(3), 0, K_NO_WAIT);

    k_thread_create(&th_app, stk_app, K_THREAD_STACK_SIZEOF(stk_app),
                    app_worker, NULL, NULL, NULL, K_PRIO_PREEMPT(3), 0, K_NO_WAIT);

    return 0;
}
