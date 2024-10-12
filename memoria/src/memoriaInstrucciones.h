#ifndef MEMORIA_INSTRUCCIONES_H_
#define MEMORIA_INSTRUCCIONES_H_

#include "utils/utils.h"
#include "utilsMemoria.h"

extern t_log* logger_memoria;
extern t_config* config_memoria;
extern t_list* procesos_memoria;

void inicializar_estructuras();
t_proceso* agregar_proceso_instrucciones(FILE* f, int pid);
char* buscar_instruccion(int pid, int tid, uint32_t program_counter);
t_proceso *buscar_proceso(t_list *lista, int pid_buscado);
t_hilo *buscar_hilo(t_proceso *proceso_padre, int tid_buscado);

// FUNCIONES PARA ELIMINAR ESTRUCTURAS

void eliminar_proceso(int pid);
void liberar_proceso(t_proceso *proceso);
void eliminar_hilo(int pid, int tid);
void liberar_hilo(t_hilo *hilo);


#endif /* MEMORIA_INSTRUCCIONES_H_ */