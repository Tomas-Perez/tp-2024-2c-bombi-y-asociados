#include "cpu.h"

t_config *config_cpu;
t_log *logger;
char *puerto_memoria;
char *puerto_escucha_dispatch;
char *puerto_escucha_interrupt;
char *ip_memoria;
char *log_level;

int socket_memoria;
int conexion_dispatch;
int main(int argc, char *argv[])
{

    levantar_config_cpu();
    logger = iniciar_logger("cpu.log", "CPU");

    // HILOS PARA CONEXIONES

    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, (void *)conectarMemoria, NULL);
    pthread_create(&t2, NULL, (void *)atenderCpuDispatch, NULL);
   // pthread_create(&t3, NULL, (void *)atenderCpuInterrupt, NULL);

    pthread_join(t2, NULL); // PRUEBAS

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

    handshake_cliente(socket_memoria, logger);

    int id_modulo = 2;
    send(socket_memoria, &id_modulo, sizeof(int), 0);
}

int atenderCpuDispatch()
{
    

	    int server_cpud = iniciar_servidor(puerto_escucha_dispatch);
	    log_info(logger, "Servidor listo para recibir al cliente");
	    conexion_dispatch = esperar_cliente(server_cpud);

	    //t_list* lista;
	    while (conexion_dispatch) {
		    int cod_op = recibir_operacion(conexion_dispatch);
		    switch (cod_op) {
		    case MENSAJE:
			   // recibir_mensaje(conexion_dispatch);
			    break;
			/*case OP_ENVIO_PCB:
        	recibir_contexto_ejecucion(conexion_dispatch); // ver que variable manda
           
			ejecutando_un_proceso = true;
			ejecutar_proceso();
			break;
		*/
		    case -1:
			    log_error(logger, "El cliente se desconecto. Terminando servidor\n");
			    return EXIT_FAILURE;
		    default:
			    log_warning(logger,"Operacion desconocida. No quieras meter la pata\n");
			    break;
		        }
	}
	return EXIT_SUCCESS;
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
