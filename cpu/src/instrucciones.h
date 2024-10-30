#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "cpu.h"
#include "utilsCpu.h"

extern t_log* logger_cpu;
extern int socket_memoria;
extern int tid;
extern bool interrupcion;

/*char *fetch();
instruccion *decode(char *inst);
instruccion *execute(instruccion *inst);*/

// Instrucciones

void set(instruccion *inst);
void sum(instruccion *inst);
void sub(instruccion *inst);
void jnz(instruccion *inst);
void read_mem(instruccion *inst);
void write_mem(instruccion *inst);
void log_instruccion(instruccion *inst);

// Syscalls

void dump_memory(instruccion *inst);
void io(instruccion *inst);
void process_create(instruccion *inst);
void thread_create(instruccion *inst);
void thread_join(instruccion *inst);
void thread_cancel(instruccion *inst);
void mutex_create(instruccion *inst);
void mutex_lock(instruccion *inst);
void mutex_unlock(instruccion *inst);
void thread_exit(instruccion *inst);
void process_exit(instruccion *inst);

uint32_t traducir_direcciones(t_registros_cpu registros_cpu, uint32_t dir_logica);
uint32_t calcular_bytes(t_registros_cpu registros_cpu, uint32_t dir_fisica);

#endif