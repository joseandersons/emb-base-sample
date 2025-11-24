#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "display.h"
#include "display_ansi.h"

static const char* get_cor(int v, int limite)
{
    int alerta = (limite * CONFIG_DISPLAY_ALERT_PERCENT) / 100;

    if (v > limite)
        return ANSI_RED;
    else if (v > alerta)
        return ANSI_YELLOW;
    else
        return ANSI_GREEN;
}

void display_update(int velocidade, const char *tipo, int limite)
{
    const char *cor = get_cor(velocidade, limite);

    printk(ANSI_CLEAR ANSI_HOME);

    printk("===== RADAR / SIMULADOR =====\n");
    printk("Tipo veículo : %s\n", tipo);
    printk("Limite       : %d km/h\n", limite);

    printk("Velocidade   : %s%d km/h%s\n",
           cor, velocidade, ANSI_RESET);

    int alerta = (limite * CONFIG_DISPLAY_ALERT_PERCENT) / 100;

    if (velocidade > limite) {
        printk("%sStatus: INFRAÇÃO%s\n", ANSI_RED, ANSI_RESET);
    }
    else if (velocidade > alerta) {
        printk("%sStatus: ALERTA%s\n", ANSI_YELLOW, ANSI_RESET);
    }
    else {
        printk("%sStatus: OK%s\n", ANSI_GREEN, ANSI_RESET);
    }
}
