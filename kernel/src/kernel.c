#include "kernel.h"
t_config* config_kernel;

char *ip_memoria;
char *puerto_memoria; 
char *ip_cpu; 
char *puerto_cpu_dispatch; 
char *puerto_cpu_interrupt; 
char *algoritmo_de_planificacion; 
int quantum; 
char *log_level; 
//t_log *logger_kernel;
int conexion_dispatch;
int conexion_interrupt;
int conexion_memoria;

int main(int argc, char* argv[]) {

   

    levantar_config_kernel();
    logger_kernel = iniciar_logger("kernel.log", "KERNEL");
    int socket = conectarMemoria();
    if(argc < 3)
    {
        log_error(logger_kernel, "Cantidad incorrecta de argumentos pasados por parametro");
        return 1;
    }
   

    pthread_t t1, t2;
    
    //inicializar_estructuras();  //ahora esta en utilsKernel.c

    pthread_create(&t1, NULL, (void *)conectarCpuDispatch, NULL);
    pthread_create(&t2, NULL, (void *)conectarCpuInterrupt, NULL);

    pthread_join(t2, NULL);

    //inicializar_hilos_planificacion();



     //./bin/kernel [archivo_pseudocodigo] [tamanio_proceso] [...args]
    int tam_proc = atoi(argv[2]);
    crear_proceso_y_pedir_memoria(argv[1], tam_proc, 0, socket);
   

    // pedir memoria para el pcb Preguntar si esta bien aca

    liberar_conexion(conexion_memoria);
    liberar_conexion(conexion_dispatch);
    liberar_conexion(conexion_interrupt);


    return 0;
}

int conectarMemoria()
{
    conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

    if (conexion_memoria <= 0)
    {
        log_error(logger_kernel, "No se pudo conectar con Memoria");
    }
    else
    {
        log_info(logger_kernel, "Conexion con memoria exitosa en socket:%i", conexion_memoria);
    }

    handshake_cliente(conexion_memoria, logger_kernel);

    int id_modulo = 1;
    send(conexion_memoria, &id_modulo, sizeof(int), 0);
    
    return conexion_memoria;
}

int conectarCpuDispatch()
{
    conexion_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch);


    if (conexion_dispatch <= 0)
    {
        log_error(logger_kernel,"DISPATCH: No se pudo establecer una conexion con la CPU");
    }
    else
    {
        log_info(logger_kernel, "DISPATCH: Conexion con CPU exitosa");
    }

    handshake_cliente(conexion_dispatch, logger_kernel);

}

int conectarCpuInterrupt()
{
    
    // Creamos una conexion hacia el servidor (socket y connect)

    conexion_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt);

    if (conexion_interrupt <= 0)
    {
        log_error(logger_kernel, "INTERRUPT: No se pudo establecer una conexion con la CPU");
    }
    else
    {
        log_info(logger_kernel, "INTERRUPT: Conexion con CPU exitosa");
    }

    handshake_cliente(conexion_interrupt, logger_kernel);

}

void levantar_config_kernel()
{ 
    config_kernel = iniciar_config("kernel.config");
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config_kernel, "IP_CPU");    
    puerto_cpu_dispatch = config_get_string_value(config_kernel, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config_kernel, "PUERTO_CPU_INTERRUPT");
    algoritmo_de_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    quantum = config_get_int_value(config_kernel, "QUANTUM");
    log_level = config_get_string_value(config_kernel, "LOG_LEVEL");
}
