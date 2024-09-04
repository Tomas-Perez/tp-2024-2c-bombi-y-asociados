#include "memoria.h"

t_config *config_memoria;
t_log *logger;

char *puerto_escucha;
char *ip_filesystem;
uint32_t tamanio_memoria;
char *path_instrucciones;
uint32_t retardo_rta;
char *esquema;
char *algoritmo_busqueda;
char *particiones; // VER SI ESTA BIEN CON *CHAR
char *log_level;
char *puerto_filesystem;

int socket_fs;

int main(int argc, char *argv[])
{
    int socket_cliente;

    levantar_config_memoria();
    logger = iniciar_logger("memoria.log", "MEMORIA");

    int socket_memoria = iniciar_servidor(puerto_escucha);

    // AVISAR QUE SE CREO EL SV Y ESTA ESPERANDO QUE SE CONECTEN
    log_info(logger, "Servidor listo para aceptar conexiones");

    // Creacion de hilos

    pthread_t t1, t2, t3;

    int socket_kernel, socket_cpu;

    pthread_create(&t3, NULL, (void *)conectarFS, &socket_fs);

    while (1)
    {
        socket_cliente = esperar_cliente(socket_memoria);

        int id_modulo;
        recv(socket_cliente, &id_modulo, sizeof(int), 0);
        // log_info(logger, "id_modulo: %i", id_modulo);

        switch (id_modulo)
        {
        case 1: // CONEXIONES EFIMERAS
            socket_kernel = socket_cliente;
            pthread_create(&t1, NULL, (void *)atenderKernel, &socket_kernel);
            break;
        case 2:
            socket_cpu = socket_cliente;
            pthread_create(&t2, NULL, (void *)atenderCpu, &socket_cpu);
            break;
        default:
            log_warning(logger, "Modulo no reconocido\n");
            break;
        }
    }

    pthread_join(t3, NULL);

    return 0;
}

int atenderCpu()
{
    log_info(logger, "Memoria conectada con CPU");
}

int atenderKernel()
{
    log_info(logger, "Memoria conectada con Kernel");
}

void levantar_config_memoria()
{
    config_memoria = config_create("memoria.config");
    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    ip_filesystem = config_get_string_value(config_memoria, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config_memoria, "PUERTO_FILESYSTEM");
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    esquema = config_get_string_value(config_memoria, "ESQUEMA");
    algoritmo_busqueda = config_get_string_value(config_memoria, "ALGORITMO_BUSQUEDA");
    particiones = config_get_string_value(config_memoria, "PARTICIONES"); // DUDOSO
    log_level = config_get_string_value(config_memoria, "LOG_LEVEL");
    tamanio_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    retardo_rta = config_get_int_value(config_memoria, "RETARDO_RESPUESTA");
}

int conectarFS()
{
    socket_fs = crear_conexion(ip_filesystem, puerto_filesystem); // creamos conexion con FS

    if (socket_fs <= 0)
    {
        log_info(logger, "No se pudo establecer una conexion con la Memoria");
    }
    else
    {
        log_info(logger, "Conexion con Memoria exitosa");
    }

    handshake_cliente(socket_fs, logger);
}