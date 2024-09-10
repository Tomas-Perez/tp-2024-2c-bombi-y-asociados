#include "filesystem.h"

t_config *config_fs;
t_log *logger_fs;

char *puerto_escucha, *mount_dir, *log_level;
uint32_t block_size,block_count,retardo_acceso_bloque;
int socket_cliente;

sem_t semaforo;

int main(int argc, char *argv[])
{

    levantar_config_fs();
    logger_fs = iniciar_logger("filesystem.log", "FILESYSTEM");

    // HILOS PARA CONEXIONES

    pthread_t t1;
    pthread_create(&t1, NULL, (void *)atenderMemoria, NULL);
    pthread_join(t1, NULL);

    return 0;
}
void atenderMemoria()
{
    char *puerto = puerto_escucha;
    int server_fs = iniciar_servidor(puerto); // socket, bind y listen
    log_info(logger_fs, "Servidor listo para recibir al cliente");
    aceptar_peticiones(server_fs);
}

void aceptar_peticiones(int socket_servidor)
{
    void* valrec;
    while (1)
    {
        sem_init(&semaforo1, 0, 0);
        pthread_t thread;
        int socket_cliente = accept(socket_servidor, NULL, NULL);
        // puts("cliente aceptado");
        pthread_create(&thread, NULL, (void *)atender_petiticiones, &socket_cliente);
        sem_wait(&semaforo1);
        pthread_join(thread, &valrec);
        enviar_mensaje((char*)valrec,socket_cliente);
        close(socket_cliente);
        // puts("siguiente hilo");
    }
}

void atender_petiticiones(int *socket)
{
    int socket_cliente = *socket;
    t_list *lista;
    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;
    // interfaz_recibida* nueva_interfaz;
    size_t bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);

    if (bytes == -1)
    {
        perror("Error al recibir handshake del cliente");
        close(socket_cliente);
    }

    if (handshake == 1)
    {
        bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
        if (bytes == -1)
        {
            perror("Error al enviar handshake de respuesta al cliente");
            close(socket_cliente);
        }
    }
    else
    {
        bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);
        if (bytes == -1)
        {
            perror("Error al enviar handshake de error al cliente");
            close(socket_cliente);
        }
    }
    int koso = 0;
    while (socket_cliente) // cambiar para registrar cada interfaz , importante guardar socket de cada interfaz
    {
        if (koso == 1)
        {
            break;
        }
        int cod_op = recibir_operacion(socket_cliente);
        switch (cod_op)
        {
        case MENSAJE:
            recibir_mensaje(socket_cliente,logger_fs);
            pthread_exit("Peticion exitosa");
            break;
        case PAQUETE:
            //lista = recibir_paquete(socket_cliente);
            puts("Me llegaron los siguientes valores:\n");
            sem_post(&semaforo1);
            koso = 1;
            break;
        case -1:
            perror(string_from_format("el cliente de socket %d se desconecto", socket_cliente));
            close(socket_cliente);
            break;
        default:
            perror("WARNING: Operacion desconocida. No quieras meter la pata");
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
