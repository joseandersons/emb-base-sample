#include <zephyr/kernel.h>
#include "display.h"

void main(void)
{
    while (1) {

        display_update(55, "Leve", 60);
        k_sleep(K_SECONDS(10000));

        display_update(68, "Leve", 60);
        k_sleep(K_SECONDS(10000));

        display_update(92, "Leve", 60);
        k_sleep(K_SECONDS(10000));

        display_update(45, "Pesado", 40);
        k_sleep(K_SECONDS(10000));

        display_update(78, "Pesado", 40);
        k_sleep(K_SECONDS(10000));
    }
}
