#include "kernel.h"
t_config_kernel configuracion_k;
int main(int argc, char* argv[]) {
//int main(){
    
    
    configuracion_k.config_kernel = iniciar_config("kernel.config");
    configuracion_k.logger_kernel = iniciar_logger("kernel.log","KERNEL");

    if(argc < 3)
    {
        log_error(configuracion_k.logger_kernel, " re mal vos");
    }
    configuracion_k.modo_de_planificacion = config_get_string_value(configuracion_k.config_kernel, "ALGORITMO_PLANIFICACION");
    //configuracion_k.grado_de_multiprogramacion = config_get_int_value(configuracion_k.config_kernel, "MULTIPROGRAMACION");

   printf("Alg de plani %s \n", configuracion_k.modo_de_planificacion);

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
