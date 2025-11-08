#pragma once
#include <time.h>
#include <zephyr/zbus/zbus.h>

struct tmsg {
    struct tm when;
};

extern const struct zbus_channel time_channel;
