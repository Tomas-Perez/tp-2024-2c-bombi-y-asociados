#include "cpu.h"

t_config *config_cpu;
t_log *logger_cpu;
char *puerto_memoria;
char *puerto_escucha_dispatch;
char *puerto_escucha_interrupt;
char *ip_memoria;
char *log_level;

bool interrupcion;
int motivo_interrupt; // ver donde declarar

int socket_memoria;
int conexion_dispatch;
int conexion_interrupt;
int main(int argc, char *argv[])
{

    levantar_config_cpu();
    logger_cpu = iniciar_logger("cpu.log", "CPU");

    // HILOS PARA CONEXIONES

    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, (void *)conectarMemoria, NULL);
    pthread_create(&t2, NULL, (void *)atenderCpuDispatch, NULL);
    pthread_create(&t3, NULL, (void *)atenderCpuInterrupt, NULL);

    pthread_join(t3, NULL); // PRUEBAS

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

    // t_list* lista;
    while (conexion_dispatch)
    {
        int cod_op = recibir_operacion(conexion_dispatch);
        switch (cod_op)
        {
        case MENSAJE:
            // recibir_mensaje(conexion_dispatch);
            break;
        case OP_ENVIO_PCB:
            recibir_pcb(conexion_dispatch); // recibe pcb y tcb para solicitar contexto a memoria
            pedir_contexto_cpu(pid, tid);
            ejecutando_un_proceso = true;
            ejecutar_proceso();
            break;
        case -1:
            log_error(logger_cpu, "El cliente se desconecto. Terminando servidor\n");
            return EXIT_FAILURE;
        default:
            log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata\n");
            break;
        }
    }
    // return EXIT_SUCCESS;
}

int atenderCpuInterrupt()
{
    int server_cpui = iniciar_servidor(puerto_escucha_interrupt);
    log_info(logger_cpu, "Servidor listo para recibir al cliente");
    conexion_interrupt = esperar_cliente(server_cpui);

    // t_list* lista;
    while (conexion_interrupt)
    {
        int cod_op = recibir_operacion(conexion_interrupt);
        switch (cod_op)
        {
        case MENSAJE:
            // recibir_mensaje(conexion_interrupt);
            break;
        case -1:
            log_error(logger_cpu, "El cliente se desconecto. Terminando servidor\n");
            return EXIT_FAILURE;
        default:
            log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata\n");
            break;
        }
    }
    // return EXIT_SUCCESS;
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

void ejecutar_proceso()
{
    while (ejecutando_un_proceso)
    {
        check_interrupt(execute(decode(fetch())));
    }
}

void check_interrupt(instruccion *inst)
{
    if (interrupcion && ejecutando_un_proceso)
    {
        //devolver_contexto_de_ejecucion(motivo_interrupt, inst);
        ejecutando_un_proceso = false;
        log_info(logger_cpu, "Proceso desalojado por motivo: %d", motivo_interrupt);
    } // hay interrupcion y un proceso en ejecucion
    interrupcion = false;
    for (int i = 0; i < list_size(inst->parametros); i++)
    {
        char *parametro = list_remove(inst->parametros, i);

        // free(parametro);
    } // liberar cada parametro de instruccion
    list_destroy(inst->parametros);
    free(inst);
}