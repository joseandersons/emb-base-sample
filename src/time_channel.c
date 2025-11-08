#include <zephyr/zbus/zbus.h>
#include "time_channel.h"

extern const struct zbus_observer app_obs;
extern const struct zbus_observer log_obs;

ZBUS_CHAN_DEFINE(time_channel,
                 struct tmsg,
                 NULL,
                 NULL,
                 ZBUS_OBSERVERS(log_obs, app_obs),
                 ZBUS_MSG_INIT(.when = (struct tm){0}));
