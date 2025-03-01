#include <utils/utils.h>

void mostrar_parametros(instruccion *inst, int cant_parametros);
char *get_termino(char *instruccion, int index);
t_list *get_parametros(char *inst, u_int8_t cant_parametros);
u_int8_t get_identificador(char *identificador_leido);
char *recibir_instruccion(int socket_cliente);
char *get_motivo(int motivo);
u_int8_t get_cant_parametros(u_int8_t identificador);
uint32_t *get_direccion_registro(char *string_registro);

// FUNCIONES CONTEXTO DE EJECUCION

void recibir_tcb(int socket);
void pedir_contexto_cpu(int pid, int tid);
void devolver_contexto_de_ejecucion(int pid, int tid);
void empaquetar_contexto(t_paquete *paquete);

// FUNCION PARA MANDAR PARAMETROS A KERNEL

void devolver_lista_instrucciones(int motivo, instruccion *info);
void  empaquetar_contexto_kl( int motivo, instruccion *info);

extern t_log* logger_cpu;
extern int tid;
extern int pid;
extern int socket_memoria;
extern t_registros_cpu registros_cpu;
extern int conexion_dispatch;
extern int motivo_interrupt;
