#include "cpu.h"

t_config *config_cpu;
t_log *logger_cpu;
char *puerto_memoria;
char *puerto_escucha_dispatch;
char *puerto_escucha_interrupt;
char *ip_memoria;
char *log_level;
int motivo_interrupt;
pthread_mutex_t m_interrupcion;
pthread_mutex_t m_ejecutando_un_proceso;

bool interrupcion;
int pid_interrupt;
int tid_interrupt;

int socket_memoria;
int conexion_dispatch;
int conexion_interrupt;

int main(int argc, char *argv[])
{

    levantar_config_cpu();
    logger_cpu = iniciar_logger("cpu.log", "CPU");
    pthread_mutex_init(&m_interrupcion, NULL);
    pthread_mutex_init(&m_ejecutando_un_proceso, NULL);
    // HILOS PARA CONEXIONES

    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, (void *)conectarMemoria, NULL);
    pthread_create(&t2, NULL, (void *)atenderCpuDispatch, NULL);
    pthread_create(&t3, NULL, (void *)atenderCpuInterrupt, NULL);

    pthread_join(t3, NULL);

    return 0;
}

int conectarMemoria()
{
    socket_memoria = crear_conexion(ip_memoria, puerto_memoria); // creamos conexion con memoria

    if (socket_memoria <= 0)
    {
        log_info(logger_cpu, "No se pudo establecer una conexion con la Memoria");
    }
    else
    {
        log_info(logger_cpu, "Conexion con Memoria exitosa");
    }

    handshake_cliente(socket_memoria, logger_cpu);

    int id_modulo = 2;
    send(socket_memoria, &id_modulo, sizeof(int), 0);
}

int atenderCpuDispatch()
{

    int server_cpud = iniciar_servidor(puerto_escucha_dispatch);
    log_info(logger_cpu, "Servidor listo para recibir al cliente");
    conexion_dispatch = esperar_cliente(server_cpud);

    while (conexion_dispatch)
    {
        int cod_op = recibir_operacion(conexion_dispatch);
        switch (cod_op)
        {
        case OP_ENVIO_TCB:
            recibir_tcb(conexion_dispatch); // recibe pcb y tcb para solicitar contexto a memoria
            pedir_contexto_cpu(pid, tid);
            pthread_mutex_lock(&m_ejecutando_un_proceso);
            ejecutando_un_proceso = true;
            pthread_mutex_unlock(&m_ejecutando_un_proceso);

            
            pthread_mutex_lock(&m_interrupcion);
            interrupcion = false;
            pthread_mutex_unlock(&m_interrupcion);
            ejecutar_proceso();
            break;
        case -1:
            log_error(logger_cpu, "El cliente se desconecto. Terminando servidorD\n");
            return EXIT_FAILURE;
        default:
            log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata\n");
            break;
        }
    }
}

int atenderCpuInterrupt()
{
    int server_cpui = iniciar_servidor(puerto_escucha_interrupt);
    log_info(logger_cpu, "Servidor listo para recibir al cliente");
    conexion_interrupt = esperar_cliente(server_cpui);

    while (conexion_interrupt)
    {
        int cod_op = recibir_operacion(conexion_interrupt);
        switch (cod_op)
        {
        case DESALOJAR_PROCESO:
            int size = 0;
            void *buffer = recibir_buffer(&size, conexion_interrupt);
            motivo_interrupt = buffer_read_uint32(buffer);
            pid_interrupt = buffer_read_uint32(buffer);
            tid_interrupt = buffer_read_uint32(buffer);
            if ((pid_interrupt == pid) && (tid_interrupt == tid))
            {
                pthread_mutex_lock(&m_interrupcion);
                interrupcion = true;
                pthread_mutex_unlock(&m_interrupcion);
            }
            free(buffer);
            break;
        case -1:
            log_error(logger_cpu, "El cliente se desconecto. Terminando servidorI\n");
            return EXIT_FAILURE;
        default:
            log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata\n");
            break;
        }
    }
}

void levantar_config_cpu()
{
    config_cpu = iniciar_config("cpu.config");
    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    log_level = config_get_string_value(config_cpu, "LOG_LEVEL");
}
