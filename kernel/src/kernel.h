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

#endif