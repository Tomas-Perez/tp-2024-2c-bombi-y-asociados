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
char *particiones;
char *log_level;
char *puerto_filesystem;

int socket_fs;
int pid, tid;
void *memoria;

void levantar_config_memoria()
{
    config_memoria = config_create("configs/memoriaPlani.config");
    //  config_memoria = config_create("configs/memoriaRC.config");
    // config_memoria = config_create("configs/memoriaPaurticionesFijas.config");
    //config_memoria = config_create("configs/memoriaParticionesDinamicas.config");
    // config_memoria = config_create("configs/memoriaFS.config");
    //config_memoria = config_create("configs/memoriaTEM.config");

    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    ip_filesystem = config_get_string_value(config_memoria, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config_memoria, "PUERTO_FILESYSTEM");
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    esquema = config_get_string_value(config_memoria, "ESQUEMA");
    algoritmo_busqueda = config_get_string_value(config_memoria, "ALGORITMO_BUSQUEDA");
    particiones = config_get_string_value(config_memoria, "PARTICIONES");
    log_level = config_get_string_value(config_memoria, "LOG_LEVEL");
    tamanio_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    retardo_rta = config_get_int_value(config_memoria, "RETARDO_RESPUESTA");
}

int main(int argc, char *argv[])
{
    int socket_cliente;
    bool esperando_clientes = true;

    levantar_config_memoria();
    inicializar_estructuras();
    logger_memoria = iniciar_logger("memoria.log", "MEMORIA");

    int socket_memoria = iniciar_servidor(puerto_escucha);

    memoria = calloc(1, tamanio_memoria); // Inicializamos memoria de usuario en 0

    if (strcmp(esquema, "FIJAS") == 0)
    {
        inicializar_particiones_fijas();
    }
    else if (strcmp(esquema, "DINAMICAS") == 0)
    {
        inicializar_particiones_dinamicas();
    }
    else
    {
        log_error(logger_memoria, "Error con ESQUEMA de las configs");
        exit(EXIT_FAILURE);
    }

    // AVISAR QUE SE CREO EL SV Y ESTA ESPERANDO QUE SE CONECTEN
    log_info(logger_memoria, "Servidor listo para aceptar conexiones");

    // Creacion de hilos

    pthread_t t1, t2, t3;

    int socket_kernel, socket_cpu;

    // pthread_create(&t3, NULL, (void *)conectarFS, &socket_fs);

    while (esperando_clientes)
    {
        socket_cliente = esperar_cliente(socket_memoria);

        int id_modulo;
        recv(socket_cliente, &id_modulo, sizeof(int), 0);

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

            esperando_clientes = false;
            close(socket_memoria);
            close(socket_cliente);
            log_warning(logger_memoria, "Modulo no reconocido\n");

            break;
        }
    }

    finalizar_estructuras_memoria();
    pthread_join(t3, NULL);

    return 0;
}

int atenderCpu(int *socket_cpu)
{
    int size = 0;
    t_buffer *buffer;
    log_info(logger_memoria, "Memoria conectada con CPU");

    while (*socket_cpu)
    {
        int cod_op = recibir_operacion(*socket_cpu);
        switch (cod_op)
        {
        case FETCH_INSTRUCCION:

            buffer = recibir_buffer((&size), *socket_cpu);

            uint32_t program_counter;

            int pid_fetch = buffer_read_uint32(buffer);
            int tid_fetch = buffer_read_uint32(buffer);
            program_counter = buffer_read_uint32(buffer);

            usleep(retardo_rta * 1000);

            char *instruccion = buscar_instruccion(pid_fetch, tid_fetch, program_counter);

            log_info(logger_memoria, "Obtener instrucción - (PID:TID) - (<%i>:<%i>) - Instrucción: <%s>", pid_fetch, tid_fetch, instruccion);

            enviar_mensaje(instruccion, *socket_cpu);
            free(buffer);

            break;
        case PEDIR_CONTEXTO:

            buffer = recibir_buffer((&size), *socket_cpu);

            int pid_PC = buffer_read_uint32(buffer);
            uint32_t tid_solicitado = buffer_read_uint32(buffer);

            log_info(logger_memoria, "Contexto <Solicitado> - (PID:TID) - (<%i>:<%i>)", pid_PC, tid_solicitado);

            free(buffer);

            usleep(retardo_rta * 1000);

            t_proceso *proceso = buscar_proceso(procesos_memoria, pid_PC);
            t_hilo *hilo = buscar_hilo(proceso, tid_solicitado);

            t_paquete *mandar_contexto = crear_paquete(PEDIR_CONTEXTO);
            empaquetar_contexto(mandar_contexto, proceso, hilo);
            enviar_paquete(mandar_contexto, *socket_cpu);
            eliminar_paquete(mandar_contexto);

            break;
        case ACTUALIZAR_CONTEXTO:

            buffer = recibir_buffer((&size), *socket_cpu);

            int pid_AC = buffer_read_uint32(buffer);
            uint32_t tid_a_actualizar = buffer_read_uint32(buffer);
            t_registros_cpu registros_a_actualizar = recibir_contexto(registros_a_actualizar, buffer);

            free(buffer);

            usleep(retardo_rta * 1000);

            t_proceso *proceso_ctx = buscar_proceso(procesos_memoria, pid_AC);
            t_hilo *hilo_ctx = buscar_hilo(proceso_ctx, tid_a_actualizar);

            actualizar_contexto_en_memoria(proceso_ctx, hilo_ctx, registros_a_actualizar);

            log_info(logger_memoria, "Contexto <Actualizado> - (PID:TID) - (<%i>:<%i>)", pid_AC, tid_a_actualizar);

            int ok = 1;
            send(*socket_cpu, &ok, sizeof(int), 0);

            break;
        case READ_MEM:

            buffer = recibir_buffer((&size), *socket_cpu);

            uint32_t tid_read = buffer_read_uint32(buffer);
            uint32_t dir_fisica_read = buffer_read_uint32(buffer);
            free(buffer);

            void *dato_a_retornar = malloc(sizeof(uint32_t));

            if (dato_a_retornar == NULL)
            {
                log_warning(logger_memoria, "Error al asignar memoria para la lectura");
                break;
            }

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(dato_a_retornar, memoria + dir_fisica_read, sizeof(uint32_t));
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(logger_memoria, "TID: <%i> - Accion: <LEER> - Direccion fisica: <%i> - Tamaño <4>", tid_read, dir_fisica_read);
            usleep(retardo_rta * 1000);

            t_paquete *paquete_dato = crear_paquete(VALOR_REGISTRO);
            agregar_a_paquete_solo(paquete_dato, dato_a_retornar, sizeof(uint32_t));
            enviar_paquete(paquete_dato, *socket_cpu);
            eliminar_paquete(paquete_dato);

            free(dato_a_retornar);

            break;
        case WRITE_MEM:

            buffer = recibir_buffer((&size), *socket_cpu);

            uint32_t tid_mem = buffer_read_uint32(buffer);
            uint32_t dir_fisica = buffer_read_uint32(buffer);
            uint32_t dato = buffer_read_uint32(buffer);
            free(buffer);

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(memoria + dir_fisica, &dato, sizeof(uint32_t));
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(logger_memoria, "TID: <%i> - Acción: <ESCRIBIR> - Dirección física: <%i> - Tamaño <4>", tid_mem, dir_fisica);
            usleep(retardo_rta * 1000);

            int confirmacion = 1;
            send(*socket_cpu, &confirmacion, sizeof(int), 0);

            break;
        default:
            // log_warning(logger_memoria, "Operacion desconocida. No quieras meter la pata\n");
            break;
        }
    }
}

int atenderKernel(int *socket_kernel)
{
    void *buffer;
    log_info(logger_memoria, "Kernel Conectado - FD del socket: <%d>", *socket_kernel);

    int cod_op = recibir_operacion(*socket_kernel);

    switch (cod_op)
    {
    case PROCESS_CREATE: // SIEMPRE EL TID VA A SER 0
        int size = 0;
        int size_PC;
        char *path_kernel;
        int confirmacion;
        buffer = recibir_buffer(&size_PC, *socket_kernel); // recibimos PCB

        if (buffer == NULL)
        {
            log_info(logger_memoria, "Error al recibir el buffer\n");
            return -1;
        }

        int pid_PC = buffer_read_uint32(buffer);
        uint32_t tamanio_proceso = buffer_read_uint32(buffer);
        path_kernel = buffer_read_string(buffer);
        free(buffer);

        t_particiones *particion_a_asignar;

        log_info(logger_memoria, "Proceso PID: %i y Tamaño: %i quiere espacio en memoria", pid_PC, tamanio_proceso);

        /*-------------------------------------------------- Particiones Fijas --------------------------------------------------*/
        if (strcmp(esquema, "FIJAS") == 0)
        {
            if (strcmp(algoritmo_busqueda, "FIRST") == 0)
            {
                particion_a_asignar = asignar_first_fit_fijas(lista_particiones, tamanio_proceso);
                if (particion_a_asignar == NULL)
                {
                    free(particion_a_asignar);
                    log_info(logger_memoria, "No hay hueco en memoria disponible");
                    confirmacion = 0;
                    send(*socket_kernel, &confirmacion, sizeof(int), 0); // Avisamos a kernel que NO pudimos reservar espacio
                    break;                                               // chequear si esta bien el return o un exit                                          
                }
            }
            else if (strcmp(algoritmo_busqueda, "BEST") == 0)
            {
                particion_a_asignar = asignar_best_fit_fijas(lista_particiones, tamanio_proceso);
                if (particion_a_asignar == NULL)
                {
                    free(particion_a_asignar);
                    log_info(logger_memoria, "No hay hueco en memoria disponible");
                    confirmacion = 0;
                    send(*socket_kernel, &confirmacion, sizeof(int), 0); // Avisamos a kernel que NO pudimos reservar espacio
                    break;                                               // chequear si esta bien el return o un exit                                        
                }
            }
            else if (strcmp(algoritmo_busqueda, "WORST") == 0)
            {
                particion_a_asignar = asignar_worst_fit_fijas(lista_particiones, tamanio_proceso);
                if (particion_a_asignar == NULL)
                {
                    free(particion_a_asignar);
                    log_info(logger_memoria, "No hay hueco en memoria disponible");
                    confirmacion = 0;
                    send(*socket_kernel, &confirmacion, sizeof(int), 0); // Avisamos a kernel que NO pudimos reservar espacio
                    break;                                               // chequear si esta bien el return o un exit                                        
                }
            }
            else
            {
                log_error(logger_memoria, "Error con ALGORITMO_BUSQUEDA de las configs");
                exit(EXIT_FAILURE);
            }
            /*--------------------------------------- PARTICIONES DINAMICAS --------------------------------------- */
        }
        else if (strcmp(esquema, "DINAMICAS") == 0)
        {
            if (strcmp(algoritmo_busqueda, "FIRST") == 0)
            {
                particion_a_asignar = asignar_first_fit_dinamicas(lista_particiones, tamanio_proceso);
                if (particion_a_asignar == NULL)
                {
                    free(particion_a_asignar);
                    log_info(logger_memoria, "No hay hueco en memoria disponible");
                    confirmacion = 0;
                    send(*socket_kernel, &confirmacion, sizeof(int), 0); // Avisamos a kernel que NO pudimos reservar espacio
                    break;                                               // chequear si esta bien el return o un exit                                         
                }
            }
            else if (strcmp(algoritmo_busqueda, "BEST") == 0)
            {
                particion_a_asignar = asignar_best_fit_dinamicas(lista_particiones, tamanio_proceso);
                if (particion_a_asignar == NULL)
                {
                    free(particion_a_asignar);
                    log_info(logger_memoria, "No hay hueco en memoria disponible");
                    confirmacion = 0;
                    send(*socket_kernel, &confirmacion, sizeof(int), 0); // Avisamos a kernel que NO pudimos reservar espacio
                    break;                                               // chequear si esta bien el return o un exit                                        
                }
            }
            else if (strcmp(algoritmo_busqueda, "WORST") == 0)
            {
                particion_a_asignar = asignar_worst_fit_dinamicas(lista_particiones, tamanio_proceso);
                if (particion_a_asignar == NULL)
                {
                    free(particion_a_asignar);
                    log_info(logger_memoria, "No hay hueco en memoria disponible");
                    confirmacion = 0;
                    send(*socket_kernel, &confirmacion, sizeof(int), 0); // Avisamos a kernel que NO pudimos reservar espacio
                    break;                                               // chequear si esta bien el return o un exit                                    
                }
            }
            else
            {
                log_error(logger_memoria, "Error con ALGORITMO_BUSQUEDA de las configs");
                exit(EXIT_FAILURE);
            }
        }

        char *path_script_completo = (char *)malloc(strlen(path_instrucciones) + strlen(path_kernel) + 1);
        if (path_script_completo == NULL)
        {
            log_info(logger_memoria, "Error al asignar memoria para path_script_completo\n");
            free(path_kernel);
            return -1;
        }
        strcpy(path_script_completo, path_instrucciones); // copia path_inst en path_script_completo
        strcat(path_script_completo, path_kernel);        // concatena path_kernel a path_script_completo

        usleep(retardo_rta * 1000);

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

        agregar_proceso_instrucciones(f, pid_PC, particion_a_asignar);
        // free(particion_a_asignar);

        log_info(logger_memoria, "Proceso <Creado> -  PID: <%i> - Tamaño: <%i>", pid_PC, tamanio_proceso);

        confirmacion = 1;
        send(*socket_kernel, &confirmacion, sizeof(int), 0); // Avisamos a kernel que pudimos reservar espacio
        break;
    case PROCESS_EXIT:
        int size_PE = 0;
        buffer = recibir_buffer(&size_PE, *socket_kernel); // recibimos pid

        if (buffer == NULL)
        {
            log_info(logger_memoria, "Error al recibir el buffer\n");
            return -1;
        }
        int pid_PE = buffer_read_uint32(buffer);
        free(buffer);

        eliminar_proceso(pid_PE); // elimina las estructuras administrativas y libera memoria en estructuras

        usleep(retardo_rta * 1000);

        confirmacion = 1;
        send(*socket_kernel, &confirmacion, sizeof(int), 0);
        break;
    case INICIAR_HILO:
        int size_hilo = 0;
        char *path_hilo;
        buffer = recibir_buffer(&size_hilo, *socket_kernel); // recibimos TCB

        if (buffer == NULL)
        {
            log_info(logger_memoria, "Error al recibir el buffer\n");
            return -1;
        }

        int pid_IH = buffer_read_uint32(buffer);
        int tid_IH = buffer_read_uint32(buffer);
        path_hilo = buffer_read_string(buffer);

        free(buffer);

        char *path_hilo_completo = (char *)malloc(strlen(path_instrucciones) + strlen(path_hilo) + 1);
        if (path_hilo_completo == NULL)
        {
            log_info(logger_memoria, "Error al asignar memoria para path_script_completo\n");
            free(path_hilo);
            return -1;
        }
        strcpy(path_hilo_completo, path_instrucciones);
        strcat(path_hilo_completo, path_hilo);

        usleep(retardo_rta * 1000);

        FILE *file_hilo;
        if (!(file_hilo = fopen(path_hilo_completo, "r")))
        { // ABRE EL ARCHIVO PARA LECTURA
            log_info(logger_memoria, "No se encontro el archivo de instrucciones\n");
            free(path_hilo_completo);
            free(path_hilo);
            return -1;
        }

        free(path_script_completo);
        free(path_hilo);

        t_proceso *proceso_padre = buscar_proceso(procesos_memoria, pid_IH);
        t_hilo *hilo_nuevo = malloc(sizeof(t_hilo));

        inicializar_hilo(proceso_padre, tid_IH, hilo_nuevo, file_hilo);

        log_info(logger_memoria, "Hilo <Creado> - (PID:TID) - (<%i>:<%i>)", proceso_padre->pid, tid_IH);

        list_add(proceso_padre->tids, hilo_nuevo);

        confirmacion = 1;
        send(*socket_kernel, &confirmacion, sizeof(int), 0);
        break;
    case THREAD_EXIT:
        int size_TE;
        buffer = recibir_buffer(&size_TE, *socket_kernel); // recibimos TID, PID

        if (buffer == NULL)
        {
            log_info(logger_memoria, "Error al recibir el buffer\n");
            return -1;
        }

        usleep(retardo_rta * 1000);

        int pid_TE = buffer_read_uint32(buffer);
        int tid_TE = buffer_read_uint32(buffer);
        free(buffer);

        eliminar_hilo(pid_TE, tid_TE);

        log_info(logger_memoria, "Hilo <Destruido> - (PID:TID) - (<%i>:<%i>)", pid_TE, tid_TE);

        confirmacion = 1;
        send(*socket_kernel, &confirmacion, sizeof(int), 0);

        break;
    case DUMP_MEMORY:
        int size_dump;
        t_buffer *buffer_dump = recibir_buffer(&size_dump, *socket_kernel);

        int socket_FS = conectarFS();
        if (buffer_dump == NULL)
        {
            log_info(logger_memoria, "Error al recibir el buffer\n");
            return -1;
        }

        usleep(retardo_rta * 1000);

        int pid_DP = buffer_read_uint32(buffer_dump);
        int tid_DP = buffer_read_uint32(buffer_dump);
        free(buffer_dump);

        log_info(logger_memoria, "Memory Dump solicitado - (PID:TID) - (<%i>:<%i>)", pid_DP, tid_DP);

        t_proceso *proceso_dump = buscar_proceso(procesos_memoria, pid_DP);

        void *contenido_memoria = malloc(proceso_dump->limite);

        if (contenido_memoria == NULL)
        {
            log_warning(logger_memoria, "Error al asignar memoria para la lectura");
            break;
        }

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(contenido_memoria, memoria + proceso_dump->base, proceso_dump->limite);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        t_paquete *paquete_dump = crear_paquete(DUMP_MEMORY);
        agregar_a_paquete_solo(paquete_dump, &pid_DP, sizeof(uint32_t));
        agregar_a_paquete_solo(paquete_dump, &tid_DP, sizeof(uint32_t));
        agregar_a_paquete_solo(paquete_dump, &proceso_dump->limite, sizeof(uint32_t));
        enviar_paquete(paquete_dump, socket_FS);
        eliminar_paquete(paquete_dump);
        send(socket_FS, contenido_memoria, proceso_dump->limite, 0);
        free(contenido_memoria);

        int confirmado_fs;
        recv(socket_FS, &confirmado_fs, sizeof(int), MSG_WAITALL);
        if (confirmado_fs == 1)
        {
            log_info(logger_memoria, "DUMP CREADO EXITOSAMENTE");
        }
        else
        {
            log_info(logger_memoria, "DUMP LLENO");
        }
        close(socket_FS);
        int confirm;

        send(*socket_kernel, &confirmado_fs, sizeof(int), 0);
        break;

    default:
        printf("Cod Op: %i", cod_op);
        break;
    }
    return 0;
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
    return socket_fs;
}
