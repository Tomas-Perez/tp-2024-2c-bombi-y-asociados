#include "cpu.h"

t_config *config_cpu;
t_log *logger;
char *puerto_memoria;
char *puerto_escucha_dispatch;
char *puerto_escucha_interrupt;
char *ip_memoria;
char *log_level;

int socket_memoria;

int main(int argc, char *argv[])
{

    levantar_config_cpu();
    logger = iniciar_logger("cpu.log", "CPU");

    // HILOS PARA CONEXIONES

    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, (void *)conectarMemoria, NULL);
    // pthread_create(&t2, NULL, (void *)atenderCpuDispatch, NULL);
    // pthread_create(&t3, NULL, (void *)atenderCpuInterrupt, NULL);

    pthread_join(t1, NULL); // PRUEBAS

    return 0;
}

int conectarMemoria()
{
    socket_memoria = crear_conexion(ip_memoria, puerto_memoria); // creamos conexion con memoria

    if (socket_memoria <= 0)
    {
        log_info(logger, "No se pudo establecer una conexion con la Memoria");
    }
    else
    {
        log_info(logger, "Conexion con Memoria exitosa");
    }

    // HANDSHAKE

    size_t bytes;

    int32_t handshake = 1;
    int32_t result;
    uint32_t resultOk = 0;
    bytes = send(socket_memoria, &handshake, sizeof(int32_t), 0);
    bytes = recv(socket_memoria, &result, sizeof(int32_t), MSG_WAITALL);

    if (result == resultOk)
    {
        log_info(logger, "Handshake exitoso entre cpu y memoria");
    }
    else
    {
        log_info(logger, "Handshake fallido entre cpu y memoria");
    }

    int id_modulo = 2;
    send(socket_memoria, &id_modulo, sizeof(int), 0);
}

void levantar_config_cpu()
{ // Tambien podriamos usar la config asi (?)
    config_cpu = iniciar_config("cpu.config");
    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    log_level = config_get_string_value(config_cpu, "LOG_LEVEL");
}
