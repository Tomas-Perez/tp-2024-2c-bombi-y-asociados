#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "cpu.h"

extern t_log* logger_cpu;
extern int socket_memoria;
extern int tid;


// Ciclo de instrucciones

char *fetch();
instruccion *decode(char *inst);
instruccion *execute(instruccion *inst);

// Parametros

void mostrar_parametros(instruccion *inst, int cant_parametros);
char *get_termino(char *instruccion, int index);
t_list *get_parametros(char *inst, u_int8_t cant_parametros);
u_int8_t get_identificador(char *identificador_leido);
char *recibir_instruccion(int socket_cliente);
char *get_motivo(int motivo);
u_int8_t get_cant_parametros(u_int8_t identificador);

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

#endif