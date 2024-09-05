#include "filesystem.h"

t_config *config_fs;
t_log *logger_fs;

char *puerto_escucha;
char *mount_dir;
uint32_t block_size;
uint32_t block_count;
uint32_t retardo_acceso_bloque;
char *log_level;
int socket_cliente;

int main(int argc, char *argv[])
{

    levantar_config_fs();
    logger_fs = iniciar_logger("filesystem.log", "FILESYSTEM");

    // HILOS PARA CONEXIONES

    int socket_fs = iniciar_servidor(puerto_escucha);

    log_info(logger_fs,"Servidor listo para recibir peticiones");

    pthread_t t1;
    void* valrec;
    while (1)
    {
        socket_cliente = esperar_cliente(socket_fs);

        //int id_modulo;
        //recv(socket_cliente, &id_modulo, sizeof(int), 0);
        // log_info(logger, "id_modulo: %i", id_modulo);

        socket_fs = socket_cliente; //preguntar despues

        pthread_create(&t1, NULL, (void *)atenderMemoria, NULL);
        pthread_join(t1, &valrec);
        enviar_mensaje((char*)valrec,socket_cliente);

    }

    return 0;
}

void atenderMemoria()
{
    log_info(logger_fs, "FS conectada con Memoria");
    	while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente,logger_fs);
            pthread_exit("Peticion exitosa");
			break;
		case PAQUETE:
            t_list* lista;
			//lista = recibir_paquete(socket_cliente);
			break;
		case -1:
			log_error(logger_fs, "el cliente se desconecto");
            break;
		default:
			log_warning(logger_fs,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
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
