/* src/display_thread.c */

#include <zephyr/kernel.h>
#include "display.h"
#include "radar_types.h"
#include "radar_threads.h"


void display_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct display_msg msg;

    while (1) {
        if (k_msgq_get(&display_queue, &msg, K_FOREVER) == 0) {
            display_update(msg.velocidade, msg.tipo);
        }
    }
}
