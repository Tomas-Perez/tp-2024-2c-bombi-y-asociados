#include "kernel.h"
t_config config_kernel;

char *ip_memoria;
char *puerto_memoria; 
char *ip_cpu; 
char *puerto_cpu_dispatch; 
char *puerto_cpu_interrupt; 
char *algoritmo_de_planificacion; 
int quantum; 
char *log_level; 


int main(int argc, char* argv[]) {
//int main(){
    


    if(argc < 3)
    {
        log_error(configuracion_k.logger_kernel, " re mal vos");
    }

    pthread_t t1, t2, t3;
    inicializar_estructuras_kernel();  //ahora esta en utils.c

    pthread_create(&t1, NULL, (void *)conectarMemoria, NULL);
    pthread_create(&t2, NULL, (void *)conectarCpuDispatch, NULL);
    pthread_create(&t3, NULL, (void *)conectarCpuInterrupt, NULL);

    pthread_join(t3, NULL);

    /*liberar_conexion(conexion_memoria);
    liberar_conexion(conexion_dispatch);
    liberar_conexion(conexion_interrupt);
    liberar_conexion(conexion_entradaSalida);*/

    return 0;
}

int conectarMemoria()
{

}

int conectarCpuDispatch()
{

}

int conectarCpuInterrupt()
{

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
