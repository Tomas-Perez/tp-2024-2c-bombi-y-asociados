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
void saludar(char *quien);

typedef enum
{
    MENSAJE,
    PAQUETE,
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

// -------------------------------- KERNEL --------------------------------
typedef struct
{
    uint32_t pid;
    int *tid;
    // t_mutex* mutex; // HACER: ver si esta ok
    //  HACER: ver que cosas agregamos
} pcb;

typedef struct
{
    int tid;
    int prioridad;
} tcb;

typedef struct
{
    t_config *config_kernel;
    t_log *logger_kernel;
    char *modo_de_planificacion;
    // int grado_de_multiprogramacion;
    int quantum;
    char *ip_memoria;
    int puerto_memoria;
    char *ip_cpu;
    int puerto_cpu_dispatch;
    int puerto_cpu_interrupt;
    char *log_level;
} t_config_kernel;

void inicializar_estructuras();

// -------------------------------- CPU --------------------------------
typedef struct
{
    t_config *config_cpu;
    t_log *logger_cpu;
    /* t_log* logger_cpu_d;
     t_log* logger_cpu_i;     VER: no se si prefieren tener solo un logger o uno para disp y otro para interr*/
    char *ip_memoria;
    int puerto_memoria;
    int puerto_escucha_dispatch;
    int puerto_escucha_interrupt;
    char *log_level;
} t_config_cpu;

// -------------------------------- MEMORIA --------------------------------
typedef struct
{
    t_config *config_memoria;
    t_log *logger_memoria;
    int puerto_escucha;
    char *ip_fs;
    int puerto_fs;
    int tam_memoria;
    char *path_instrucciones;
    int retardo_respuesta;
    char *esquema;
    char *alg_busqueda;
    char **particiones; // VER: no se si estoy flasheando
    char *log_level;
} t_config_memoria;

// -------------------------------- FILESYSTEM --------------------------------
typedef struct
{
    t_config *config_fs;
    t_log *logger_fs;
    int puerto_escucha;
    char *mount_dir;
    int block_size;
    int block_count;
    int retardo_acc_bloque;
    char *log_level;
} t_config_fs;

// -------------------------------- LOGGER --------------------------------
t_log *iniciar_logger(char *nombreLog, char *proceso);

// -------------------------------- CONFIG --------------------------------
t_config *iniciar_config(char *archivo);

// -------------------------------- CONEXIONES: CLIENTE --------------------------------
void liberar_conexion(int socket_cliente);
int crear_conexion(char *ip, char *puerto);


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

#endif
