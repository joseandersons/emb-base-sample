// include/radar_types.h
#ifndef RADAR_TYPES_H
#define RADAR_TYPES_H

#include <stdint.h>
#include "display.h"  // para enum tipo_veiculo_t

enum STATE {
    IDLE = 0,
    READING,
    DONE
};

enum TYPE {
    LEVE = 0,
    PESADO
};

enum FROM {
    SENSOR_0 = 0,
    SENSOR_1,
    TIMER
};

struct data {
    union item {
        enum FROM from;
        enum TYPE type;
    } item;

    union value {
        uint32_t speed;
        uint32_t timestamp;
    } value;

    int eixos;
};

struct display_msg {
    int velocidade;
    enum tipo_veiculo_t tipo;
};


extern struct k_msgq display_queue;
extern struct k_msgq sensor_queue;
extern struct k_msgq vehicle_queue;

extern bool plate_is_valid(const char *raw);
extern enum STATE radar_state;


#endif
