#include "memoria.h"

t_config *config_memoria;
t_log *logger_memoria;

char *puerto_escucha;
char *ip_filesystem;
uint32_t tamanio_memoria;
char *path_instrucciones;
uint32_t retardo_rta;
char *esquema;
char *algoritmo_busqueda;
char *particiones; // VER SI ESTA BIEN CON *CHAR
char *log_level;
char *puerto_filesystem;

int socket_fs;

int main(int argc, char *argv[])
{
    int socket_cliente;

    levantar_config_memoria();
    logger_memoria = iniciar_logger("memoria.log", "MEMORIA");

    int socket_memoria = iniciar_servidor(puerto_escucha);

    // AVISAR QUE SE CREO EL SV Y ESTA ESPERANDO QUE SE CONECTEN
    log_info(logger_memoria, "Servidor listo para aceptar conexiones");

    // Creacion de hilos

    pthread_t t1, t2, t3;

    int socket_kernel, socket_cpu;

    pthread_create(&t3, NULL, (void *)conectarFS, &socket_fs);

    while (1)
    {
        socket_cliente = esperar_cliente(socket_memoria);

        int id_modulo;
        recv(socket_cliente, &id_modulo, sizeof(int), 0);
        // log_info(logger, "id_modulo: %i", id_modulo);

        switch (id_modulo)
        {
        case 1: // CONEXIONES EFIMERAS
            socket_kernel = socket_cliente;
            pthread_create(&t1, NULL, (void *)atenderKernel, &socket_kernel);
            break;
        case 2:
            socket_cpu = socket_cliente;
            pthread_create(&t2, NULL, (void *)atenderCpu, &socket_cpu);
            break;
        default:
            log_warning(logger_memoria, "Modulo no reconocido\n");
            break;
        }
    }

    pthread_join(t3, NULL);

    return 0;
}

int atenderCpu()
{
    log_info(logger_memoria, "Memoria conectada con CPU");
}

int atenderKernel(int *socket_kernel)
{
    void *buffer;
    log_info(logger_memoria, "Memoria conectada con Kernel");

    int cod_op = recibir_operacion(*socket_kernel);

    switch (cod_op) 
    {
        case PROCESS_CREATE: // SIEMPRE EL TID VA A SER 0
            int size = 0;
            char *path_kernel;
			buffer = recibir_buffer(&size, *socket_kernel); // recibimos PCB

			if (buffer == NULL)
			{
				log_info(logger_memoria, "Error al recibir el buffer\n");
				return -1;
			}

            uint32_t pid = buffer_read_uint32(buffer);
            uint32_t tamanio_proceso = buffer_read_uint32(buffer);

            /*if (tamanio_proceso > memoria_disponible) { // DEPENDE DEL TIPO DE MEMORIA Y DEL ESQUEMA, VEREMOS MAS ADELANTE
                // Devolver que no hay espacio disponible (?)
            }*/

            uint32_t size_path = buffer_read_uint32(buffer);

            path_kernel = malloc(size_path + 1); // asignamos memoria, +1 para el carácter nulo
			if (path_kernel == NULL)
			{
				log_info(logger_memoria, "Error al asignar memoria para path_kernel\n");
				return -1;
			}

            path_kernel = buffer_read_string(buffer, size_path);
            path_kernel[size_path] = '\0'; // aseguramos que la cadena termine en un carácter nulo

            char *path_script_completo = (char *)malloc(strlen(path_instrucciones) + strlen(path_kernel) + 1);
			if (path_script_completo == NULL)
			{
				log_info(logger_memoria, "Error al asignar memoria para path_script_completo\n");
				free(path_kernel);
				return -1;
			}
			strcpy(path_script_completo, path_instrucciones);   // copia path_inst en path_script_completo
			strcat(path_script_completo, path_kernel); // concatena path_kernel a path_script_completo

			//usleep(retardo_memoria() * 1000);

			printf("PATH: %s\n", path_script_completo); // debería mostrar el path completo, chequear que muestre bien

            FILE *f;
			if (!(f = fopen(path_script_completo, "r")))
			{ // ABRE EL ARCHIVO PARA LECTURA
				log_info(logger_memoria, "No se encontro el archivo de instrucciones\n");
				free(path_script_completo);
				free(path_kernel);
				return -1;
			}

			free(path_script_completo);
			free(path_kernel);
            free(buffer);

            agregar_proceso_instrucciones(f, pid); // Ver como remodelamos esta funcion para este TP

            bool confirmacion = true;
            send(*socket_kernel, &confirmacion, sizeof(bool), 0); // Avisamos a kernel que pudimos reservar espacio
            break;
        default:
            log_warning(logger_memoria, "Operacion desconocida. No quieras meter la pata\n");
			printf("Cod Op: %i", cod_op);
			break;
    }

}

void levantar_config_memoria()
{
    config_memoria = config_create("memoria.config");
    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    ip_filesystem = config_get_string_value(config_memoria, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config_memoria, "PUERTO_FILESYSTEM");
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    esquema = config_get_string_value(config_memoria, "ESQUEMA");
    algoritmo_busqueda = config_get_string_value(config_memoria, "ALGORITMO_BUSQUEDA");
    particiones = config_get_string_value(config_memoria, "PARTICIONES"); // DUDOSO
    log_level = config_get_string_value(config_memoria, "LOG_LEVEL");
    tamanio_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    retardo_rta = config_get_int_value(config_memoria, "RETARDO_RESPUESTA");
}

int conectarFS()
{
    socket_fs = crear_conexion(ip_filesystem, puerto_filesystem); // creamos conexion con FS

    if (socket_fs <= 0)
    {
        log_info(logger_memoria, "No se pudo establecer una conexion con el FS");
    }
    else
    {
        log_info(logger_memoria, "Conexion con FS exitosa");
    }

    handshake_cliente(socket_fs, logger_memoria);
}