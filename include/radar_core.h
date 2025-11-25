#ifndef RADAR_CORE_H
#define RADAR_CORE_H

enum tipo_veiculo_t {
    VEICULO_LEVE,
    VEICULO_PESADO
};

enum status_t {
    STATUS_OK,
    STATUS_ALERTA,
    STATUS_INFRACAO
};

struct radar_result_t {
    int limite;
    enum status_t status;
};

struct radar_result_t radar_analisar(int velocidade, enum tipo_veiculo_t tipo);

#endif
