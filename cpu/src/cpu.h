#ifndef CPU_H_
#define CPU_H_

#include <utils/utils.h>

#define PUERTO_ESCUCHA_D "8006"
#define PUERTO_ESCUCHA_I "8007"

void levantar_config_cpu();

// PROTOTIPOS HILOS DE CONEXIONES 

int atenderCpuDispatch();
int atenderCpuInterrupt();
int conectarMemoria();

#endif