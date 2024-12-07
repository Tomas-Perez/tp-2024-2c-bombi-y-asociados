#include "kernel.h"
/*t_config* config_kernel;

char *ip_memoria;
char *puerto_memoria; 
char *ip_cpu; 
char *puerto_cpu_dispatch; 
char *puerto_cpu_interrupt; 
char *algoritmo_de_planificacion;  
int quantum; 
char *log_level; */
//t_log *logger_kernel;
int conexion_dispatch;
int conexion_interrupt;
int conexion_memoria;

int main(int argc, char* argv[]) {

    levantar_config_kernel();
    logger_kernel = iniciar_logger("kernel.log", "KERNEL");
    
    /*if(argc < 3)
    {
        log_error(logger_kernel, "Cantidad incorrecta de argumentos pasados por parametro");
        return 1;
    }*/
   
    argv[1] = "PRUEBA_FS";
    argv[2] = "8";

    pthread_t t1, t2;
    
    inicializar_estructuras_kernel();  //ahora esta en utilsKernel.c

    pthread_create(&t1, NULL, (void *)conectarCpuDispatch, NULL);
    pthread_create(&t2, NULL, (void *)conectarCpuInterrupt, NULL);

    pthread_join(t2, NULL);

    

     //./bin/kernel [archivo_pseudocodigo] [tamanio_proceso] [...args]

    int socket = conectarMemoria();
    int tam_proc = atoi(argv[2]);
    printf("tam proc %d archivo: %s\n", tam_proc, argv[1]);

    sem_post(&binario_corto_plazo);
    pcb* proceso_nuevo = crear_pcb(0, argv[1], tam_proc, socket);
    inicializar_hilos_planificacion();
    printf("liiiiiiiiiiiiiiiibre sooooooy? o es algo que me quieren hacer creer?\n");


    liberar_conexion(conexion_memoria);
    liberar_conexion(conexion_dispatch);
    liberar_conexion(conexion_interrupt);

    log_destroy(logger_kernel);
    config_destroy(config_kernel);
    finalizar_estructuras_kernel();
    
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


