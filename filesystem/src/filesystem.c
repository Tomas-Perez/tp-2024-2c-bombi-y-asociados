#include "filesystem.h"

t_config *config_fs;
t_log *logger;

char *puerto_escucha;
char *mount_dir;
uint32_t block_size;
uint32_t block_count;
uint32_t retardo_acceso_bloque;
char *log_level;

int main(int argc, char* argv[]) {

    levantar_config_fs();
    logger = iniciar_logger("filesystem.config", "FILESYSTEM");

    // HILOS PARA CONEXIONES

    int socket_fs = iniciar_servidor(puerto_escucha);

    pthread_t t1;

    pthread_create(&t1, NULL, (void *)atenderMemoria, NULL);
    
    return 0;
}

void atenderMemoria() {
        log_info(logger, "FS conectada con Memoria");
}

void levantar_config_fs() {
    config_fs = iniciar_config("filesystem.config");
    puerto_escucha = config_get_string_value(logger, "PUERTO_ESCUCHA");
    mount_dir = config_get_string_value(logger, "MOUNT_DIR");
    log_level = config_get_string_value(logger, "LOG_LEVEL");
    block_size = config_get_int_value(logger, "BLOCK_SIZE");
    block_count = config_get_int_value(logger, "BLOCK_COUNT");
    retardo_acceso_bloque = config_get_int_value(logger, "RETARDO_ACCESO_BLOQUE");
}
