#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <time.h>
#include <commons/bitarray.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <commons/temporal.h>
#include <semaphore.h>

/**
 * @brief Imprime un saludo por consola
 * @param quien Módulo desde donde se llama a la función
 * @return No devuelve nada
 */

typedef enum
{
    MENSAJE,
    PAQUETE,
    SET,
    READ_MEM,
    WRITE_MEM,
    SUM,
    SUB,
    JNZ,
    LOG,
    DUMP_MEMORY,
    IO,
    PROCESS_CREATE,
    THREAD_CREATE,
    THREAD_JOIN,
    THREAD_CANCEL,
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
    THREAD_EXIT,
    PROCESS_EXIT,
    NO_RECONOCIDO,
    FETCH_INSTRUCCION,
    SYSCALL,
    SUCCESS,
    INICIAR_HILO,
    DESALOJAR_PROCESO,
    OP_ENVIO_PCB,
    PEDIR_CONTEXTO,
    ACTUALIZAR_CONTEXTO,
    OP_ENVIO_TCB
} op_code;

typedef struct
{
    uint32_t size;   // Tamaño total del buffer
    void *stream;    // Puntero al inicio del buffer
    uint32_t offset; // Posición actual del buffer
} t_buffer;

typedef struct
{
    op_code codigo_operacion;
    t_buffer *buffer;
} t_paquete;

typedef struct
{
    int cod_op;
    t_list *parametros;
} t_comando;

typedef struct
{
    int identificador; // dice que instruccion es
    int cant_parametros;
    t_list* parametros; // lista de parametros (separados entre si), sin contar el identificador
} instruccion;

// -------------------------------- CPU --------------------------------
typedef struct
{
    uint32_t PC;
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
    uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
} t_registros_cpu;

// -------------------------------- KERNEL --------------------------------
typedef struct
{
    uint32_t pid;
    int contador_tid;
    bool mem_asignada;
    t_list *lista_tcb;
    int tam_proc;
    char* path_proc;
    t_list* lista_mutex_proc;
} pcb;

typedef struct
{
    int tid;
    int prioridad;
    pcb* pcb_padre_tcb;
    t_registros_cpu registros_cpu;
    t_list* lista_mutex;
} tcb;

typedef struct 
{
    char* nombre;
    bool disponibilidad;
    tcb* hilo_poseedor;
    t_list* bloqueados_por_mutex;

} mutex_k;
typedef struct 
{
    int prioridad;
    t_list* hilos_asociados;
    pthread_mutex_t m_lista_prioridad;

} nivel_prioridad; 
void inicializar_estructuras();

// -------------------------------- MEMORIA --------------------------------

typedef struct
{
    uint32_t pid;
    uint32_t base;
    uint32_t limite;
    t_list *tids; // lista de t_hilo
} t_proceso;

typedef struct
{
    uint32_t tid;
    uint32_t pid_padre; // nose si hace falta
    t_list *instrucciones;
    t_registros_cpu registros_hilo;
} t_hilo;

typedef struct
{
    uint32_t pid;
    uint32_t tid;
    uint32_t base;
    uint32_t limite;
    t_registros_cpu registros;
} t_contexto_ejecucion; // ENTRE CPU Y MEM

// -------------------------------- FILESYSTEM --------------------------------

// -------------------------------- LOGGER --------------------------------
// extern t_log* logger;
t_log *iniciar_logger(char *nombreLog, char *proceso);

// -------------------------------- CONFIG --------------------------------
t_config *iniciar_config(char *archivo);

// -------------------------------- CONEXIONES: CLIENTE --------------------------------
void liberar_conexion(int socket_cliente);
int crear_conexion(char *ip, char *puerto);
void handshake_cliente(int socket_cliente, t_log *logger);

// -------------------------------- CONEXIONES: SERVIDOR --------------------------------

int iniciar_servidor(char *puerto);
int esperar_cliente(int socket_servidor);

// -------------------------------- ENVIO INFO --------------------------------

void buffer_add_uint32(t_buffer *buffer, uint32_t data);
uint32_t buffer_read_uint32(t_buffer *buffer);
void buffer_add_uint8(t_buffer *buffer, uint8_t data);
uint8_t buffer_read_uint8(t_buffer *buffer);
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);
char *buffer_read_string(t_buffer *buffer, uint32_t *length);

int recibir_operacion(int socket_cliente);
void *recibir_buffer(int *size, int socket_cliente);

void recibir_mensaje(int socket_cliente, t_log *logger);
void enviar_mensaje(char *mensaje, int socket_cliente);
void *serializar_paquete(t_paquete *paquete, int bytes);
void eliminar_paquete(t_paquete *paquete);
t_paquete *crear_paquete(int codigo_operacion);
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);
void agregar_a_paquete_solo(t_paquete *paquete, void *valor, int bytes);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
t_buffer *crear_buffer();

#endif
