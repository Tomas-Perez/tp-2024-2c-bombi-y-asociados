#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>
#include "memoriaInstrucciones.h"

#define PUERTO_MEMORIA "8002"


void levantar_config_memoria();
void inicializar_particiones_fijas();
void inicializar_particiones_dinamicas();
char* eliminar_corchetes(char* cad);

int atenderCpu();
int atenderKernel();
int conectarFS();

// FUNCIONES PARA MANEJO DE PARTICIONES

t_particiones *asignar_first_fit(t_list *lista, uint32_t tamanio);
//int asignar_best_fit(t_list *lista, t_proceso *proceso, uint32_t tamanio);
//int asignar_worst_fit(t_list *lista, t_proceso *proceso, uint32_t tamanio);

extern t_list *particiones_fijas;
extern t_list *particiones_dinamicas;


#endif