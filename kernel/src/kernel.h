#ifndef KERNEL_H_
#define KERNEL_H_
#include "planificador.h"

void levantar_config_kernel();

int conexion_dispatch;
int conexion_interrupt;
int conexion_memoria;

int conectarMemoria();
int conectarCpuDispatch();
int conectarCpuInterrupt();
void levantar_config_kernel();

extern char *ip_memoria;
extern char *puerto_memoria; 
extern char *ip_cpu; 
extern char *puerto_cpu_dispatch; 
extern char *puerto_cpu_interrupt; 
extern char *algoritmo_de_planificacion; 
extern int quantum; 
extern char *log_level; 
//extern t_log *logger_kernel;
#endif