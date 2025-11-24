#include <zephyr/sys/printk.h>
#include "display.h"
#include "display_ansi.h"
#include "radar_core.h"

void display_update(int velocidade, enum tipo_veiculo_t tipo)
{
    struct radar_result_t r = radar_analisar(velocidade, tipo);

    printk(ANSI_CLEAR ANSI_HOME);

    printk("===== RADAR / SIMULADOR =====\n");

    printk("Tipo veículo : %s\n", 
        tipo == VEICULO_LEVE ? "Leve" : "Pesado");

    printk("Limite       : %d km/h\n", r.limite);

    const char *cor =
        (r.status == STATUS_OK) ? ANSI_GREEN :
        (r.status == STATUS_ALERTA) ? ANSI_YELLOW :
        ANSI_RED;

    printk("Velocidade   : %s%d km/h%s\n",
            cor, velocidade, ANSI_RESET);

    if (r.status == STATUS_OK)
        printk("%sStatus: OK%s\n", ANSI_GREEN, ANSI_RESET);
    else if (r.status == STATUS_ALERTA)
        printk("%sStatus: ALERTA%s\n", ANSI_YELLOW, ANSI_RESET);
    else
        printk("%sStatus: INFRAÇÃO%s\n", ANSI_RED, ANSI_RESET);
}
