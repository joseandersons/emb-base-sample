#include <zephyr/kernel.h>
#include "display.h"
#include "radar_core.h"

void main(void)
{
    while (1) {
        display_update(75, VEICULO_LEVE);
        k_sleep(K_MSEC(10000));

        display_update(45, VEICULO_PESADO);
        k_sleep(K_MSEC(10000));

        display_update(82, VEICULO_PESADO);
        k_sleep(K_MSEC(10000));
    }
}
