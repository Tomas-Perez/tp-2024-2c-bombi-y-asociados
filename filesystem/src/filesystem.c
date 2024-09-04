#include "filesystem.h"

t_config *config_fs;
t_log *logger;

char *puerto_escucha;
char *mount_dir;
uint32_t block_size;
uint32_t block_count;
uint32_t retardo_acceso_bloque;
char *log_level;

int main(int argc, char *argv[])
{
    int socket_cliente;

    levantar_config_fs();
    logger = iniciar_logger("filesystem.log", "FILESYSTEM");

    // HILOS PARA CONEXIONES

    int socket_fs = iniciar_servidor(puerto_escucha);

    log_info(logger,"Servidor listo para recibir peticiones");

    pthread_t t1;

    while (1)
    {
        socket_cliente = esperar_cliente(socket_fs);

        //int id_modulo;
        //recv(socket_cliente, &id_modulo, sizeof(int), 0);
        // log_info(logger, "id_modulo: %i", id_modulo);
        socket_fs = socket_cliente;
        pthread_create(&t1, NULL, (void *)atenderMemoria, NULL);
    }

    return 0;
}

void atenderMemoria()
{
    log_info(logger, "FS conectada con Memoria");
}

void levantar_config_fs()
{
    config_fs = config_create("filesystem.config");
    puerto_escucha = config_get_string_value(config_fs, "PUERTO_ESCUCHA");
    mount_dir = config_get_string_value(config_fs, "MOUNT_DIR");
    log_level = config_get_string_value(config_fs, "LOG_LEVEL");
    block_size = config_get_int_value(config_fs, "BLOCK_SIZE");
    block_count = config_get_int_value(config_fs, "BLOCK_COUNT");
    retardo_acceso_bloque = config_get_int_value(config_fs, "RETARDO_ACCESO_BLOQUE");
}
