#ifndef MEMORIA_INSTRUCCIONES_H_
#define MEMORIA_INSTRUCCIONES_H_

#include "utils/utils.h"
#include "utilsMemoria.h"

extern t_log* logger_memoria;
extern t_config* config_memoria;
extern t_list* procesos_memoria;

void inicializar_estructuras();
t_proceso* agregar_proceso_instrucciones(FILE* f, int pid);
/*char* buscar_instruccion_proceso(int pid, uint32_t program_counter);
t_proceso *buscar_proceso(t_list *lista, int pid_buscado);
void eliminar_proceso(int pid);
void liberar_proceso(t_proceso *proceso);*/

#endif /* MEMORIA_INSTRUCCIONES_H_ */