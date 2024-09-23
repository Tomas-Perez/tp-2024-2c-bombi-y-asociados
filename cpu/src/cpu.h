#ifndef CPU_H_
#define CPU_H_

#include <utils/utils.h>

#define PUERTO_ESCUCHA_D "8006"
#define PUERTO_ESCUCHA_I "8007"

extern int tid;
extern int pid;
extern bool ejecutando_un_proceso;
extern t_registros_cpu registros_cpu;

void levantar_config_cpu();

// PROTOTIPOS HILOS DE CONEXIONES 

int atenderCpuDispatch();
int atenderCpuInterrupt();
int conectarMemoria();

void ejecutar_proceso();
char* fetch();
instruccion* decode(char*);
instruccion* execute(instruccion* inst);
void check_interrupt(instruccion* inst);


#endif