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
t_log *logger_kernel;

int main(int argc, char* argv[]) {
//int main(){
    

    levantar_config_kernel();
    logger_kernel = iniciar_logger("kernel.log", "KERNEL");

    if(argc < 3)
    {
        log_error(logger_kernel, " re mal vos");
    }

    pthread_t t1, t2, t3;
    //inicializar_estructuras_kernel();  //ahora esta en utils.c

    pthread_create(&t1, NULL, (void *)conectarMemoria, NULL);
    pthread_create(&t2, NULL, (void *)conectarCpuDispatch, NULL);
    pthread_create(&t3, NULL, (void *)conectarCpuInterrupt, NULL);

    pthread_join(t3, NULL);

    liberar_conexion(conexion_memoria);
    liberar_conexion(conexion_dispatch);
    liberar_conexion(conexion_interrupt);


    return 0;
}

int conectarMemoria()
{
    conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
}

int conectarCpuDispatch()
{
    printf("ip_cpu %s puerto_cpu_dispatch %s\n",ip_cpu, puerto_cpu_dispatch);
    conexion_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch);


    if (conexion_dispatch <= 0)
    {
        log_error(logger_kernel, " DISPATCH: No se pudo establecer una onexion con la CPU\n");
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
        log_error(logger_kernel, "INTERRUPT: No se pudo establecer unaconexion con la CPU");
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
