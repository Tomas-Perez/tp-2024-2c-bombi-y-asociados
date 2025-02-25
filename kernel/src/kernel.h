#ifndef KERNEL_H_
#define KERNEL_H_
#include "planificador.h"

void levantar_config_kernel();



int conectarMemoria();
int conectarCpuDispatch();
int conectarCpuInterrupt();

extern char *ip_memoria;
extern char *puerto_memoria; 
extern char *ip_cpu; 
extern char *puerto_cpu_dispatch; 
extern char *puerto_cpu_interrupt; 
extern double quantum; 
extern char *log_level; 

#endif