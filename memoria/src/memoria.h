#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>
#include "memoriaInstrucciones.h"

#define PUERTO_MEMORIA "8002"


void levantar_config_memoria();

int atenderCpu();
int atenderKernel();
int conectarFS();

// FUNCIONES PARA MANEJO DE PARTICIONES

extern t_list *particiones_fijas;


void inicilizar_particiones_fijas();
char* eliminar_corchetes(char* cad);


#endif