#include "radar_core.h"
#include <zephyr/kernel.h>

struct radar_result_t radar_analisar(int velocidade, enum tipo_veiculo_t tipo)
{
    struct radar_result_t r;

    if (tipo == VEICULO_LEVE)
        r.limite = CONFIG_DISPLAY_LIMITE_LEVE;
    else
        r.limite = CONFIG_DISPLAY_LIMITE_PESADO;

    int alerta = (r.limite * CONFIG_DISPLAY_ALERT_PERCENT) / 100;

    if (velocidade > r.limite)
        r.status = STATUS_INFRACAO;
    else if (velocidade > alerta)
        r.status = STATUS_ALERTA;
    else
        r.status = STATUS_OK;

    return r;
}
