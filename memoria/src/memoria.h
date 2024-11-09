#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>
#include "memoriaInstrucciones.h"

#define PUERTO_MEMORIA "8002"


void levantar_config_memoria();

int atenderCpu();
int atenderKernel();
int conectarFS();

extern t_list *particiones_fijas;
extern t_list *particiones_dinamicas;
extern pthread_mutex_t mutex_espacio_usuario;


#endif