#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/dns_resolve.h>
#include "wifi.h"

LOG_MODULE_REGISTER(netlink, LOG_LEVEL_INF);

static struct k_sem ip_sem;
static struct net_mgmt_event_callback ip_cb;
bool net_up;

static void ip_evt(struct net_mgmt_event_callback *cb, uint64_t evt, struct net_if *iface)
{
    if (evt == NET_EVENT_IPV4_ADDR_ADD) {
        k_sem_give(&ip_sem);
    }
}

static int wifi_join(const char *ssid, const char *psk)
{
    struct net_if *iface = net_if_get_default();
    struct wifi_connect_req_params p = {0};
    p.ssid = ssid;
    p.ssid_length = strlen(ssid);
    p.psk = psk;
    p.psk_length = strlen(psk);
    p.security = WIFI_SECURITY_TYPE_PSK;
    p.channel = WIFI_CHANNEL_ANY;
    return net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &p, sizeof(p));
}

int net_autostart(void)
{
    k_sem_init(&ip_sem, 0, 1);
    net_mgmt_init_event_callback(&ip_cb, ip_evt, NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ip_cb);

    if (wifi_join(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWD) != 0) {
        return -EIO;
    }

    if (k_sem_take(&ip_sem, K_SECONDS(30)) != 0) {
        return -ETIMEDOUT;
    }

    struct zsock_addrinfo hints = {0}, *res = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (zsock_getaddrinfo("google.com", "80", &hints, &res) != 0) {
        return -EIO;
    }
    zsock_freeaddrinfo(res);

    net_up = true;
    return 0;
}

SYS_INIT(net_autostart, APPLICATION, 50);
