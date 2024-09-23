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

void recibir_pcb(int socket);
void pedir_contexto_cpu(int pid, int tid);

extern t_log* logger_cpu;
extern int tid;
extern int pid;
extern int socket_memoria;
extern t_registros_cpu registros_cpu;
