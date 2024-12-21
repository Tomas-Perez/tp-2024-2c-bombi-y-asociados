// quizas no esta en la mejor carpeta
/*#include <stdio.h>
#include<stdlib.h>*/

#include "utilsKernel.h"

t_config *config_kernel;
t_log *logger_kernel;
char *ip_memoria;
char *puerto_memoria;
char *ip_cpu;
char *puerto_cpu_dispatch;
char *puerto_cpu_interrupt;
char *algoritmo_de_planificacion;
double quantum;
int syscall_solicitada;
char *log_level;
int syscall_replanificadora;
int contador;
pthread_mutex_t m_lista_mutex;


//----------------------------------------------------------------
// --------------------------- indice ---------------------------
//----------------------------------------------------------------
// 1) Archivo inicial
// 2) Inicializar
// 3) Crear
// 4) Funciones planificador
// 4) Pedidos memoria
// 5) Parametros
// 6) Iniciar hilos
// 7) Finalizar
// 8) Buscar
// 9) Dump
// 10) Mutex
// 11) Funciones auxiliares

// --------------------------- Archivo inicial ---------------------------

#define MAX_LENGTH 1000 // maximo tamaño de la cadena
#define MAX_LINES 10

char *generar_path_archivo(char *nombre_archivo)
{
    char *path_archivo = nombre_archivo;
    char *path_archivo_inicio = "/home/utnso/";
    char *path_archivo_completo = (char *)malloc(strlen(path_archivo_inicio) + strlen(path_archivo) + 1);
    strcpy(path_archivo_completo, path_archivo_inicio);
    strcat(path_archivo_completo, path_archivo);
    printf("%s\n", path_archivo_completo);

    return path_archivo_completo;
}

// --------------------------- Inicializar ---------------------------

void levantar_config_kernel()
{
    //config_kernel = iniciar_config("configs/kernelFS.config");
    // config_kernel = iniciar_config("configs/kernelRC.config");
    //config_kernel = iniciar_config("configs/kernelParticionesDinamicas.config");
    //config_kernel = iniciar_config("configs/kernelParticionesFijas.config");
    config_kernel = iniciar_config("configs/kernelPlani.config");
   // config_kernel = iniciar_config("configs/kernelTEM.config");

    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config_kernel, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config_kernel, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config_kernel, "PUERTO_CPU_INTERRUPT");
    algoritmo_de_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    quantum = config_get_double_value(config_kernel, "QUANTUM");
    //printf("quantum %d  ", quantum);
    log_level = config_get_string_value(config_kernel, "LOG_LEVEL");
}

void inicializar_registros(tcb *hilo)
{
    hilo->registros_cpu.AX = 0;
    hilo->registros_cpu.BX = 0;
    hilo->registros_cpu.CX = 0;
    hilo->registros_cpu.DX = 0;
    hilo->registros_cpu.EX = 0;
    hilo->registros_cpu.FX = 0;
    hilo->registros_cpu.GX = 0;
    hilo->registros_cpu.HX = 0;
    hilo->registros_cpu.PC = 0;
}

void inicializar_estructuras_kernel()
{
    id_counter = 0;
    syscall_replanificadora = 0;
    quantum_restante = 1;
    int contador = 0;
    syscall_solicitada = 0;

    // mutexs
    pthread_mutex_init(&m_hilo_en_ejecucion, NULL);
    pthread_mutex_init(&m_lista_de_ready, NULL);
    // pthread_mutex_init(&m_lista_bloqueados,NULL);
    pthread_mutex_init(&m_regreso_de_cpu, NULL);
    pthread_mutex_init(&m_hilo_a_ejecutar, NULL); // TO DO destroy de los mutex
    pthread_mutex_init(&m_lista_procesos_new, NULL);
    pthread_mutex_init(&m_lista_multinivel, NULL);
    pthread_mutex_init(&m_lista_finalizados, NULL);
    pthread_mutex_init(&m_quantum_restante, NULL);
    // pthread_mutex_init(&m_lista_prioridad, NULL);
    pthread_mutex_init(&m_lista_io, NULL);
    pthread_mutex_init(&m_bloqueados_por_dump, NULL);
    pthread_mutex_init(&m_syscall_replanificadora, NULL);
    pthread_mutex_init(&m_contador, NULL);
    pthread_mutex_init(&m_lista_mutex, NULL);

    // semaforos
    sem_init(&finalizo_un_proc, 0, 0);
    sem_init(&hilos_en_exit, 0, 0);
    sem_init(&hilos_en_ready, 0, 0);
    sem_init(&binario_corto_plazo, 0, 0);
    sem_init(&bin_dispatch, 0, 1);
    sem_init(&bin_memoria, 0, 1);
   
    // cola de procesos
    lista_de_ready = list_create();
    lista_procesos_new = list_create();
    lista_multinivel = list_create();
    lista_finalizados = list_create();
    lista_io = list_create();
    bloqueados_por_dump = list_create();
}

void inicializar_hilos_planificacion()
{

    pthread_t hilo_plani_corto, hilo_exitt;

    pthread_create(&hilo_plani_corto, NULL, (void *)planificador_corto_plazo, NULL);
    pthread_create(&hilo_exitt, NULL, (void *)hilo_exit, NULL);
    // pthread_create(&hilo_syscall, NULL, (void*) atender_syscall, NULL);

    pthread_join(hilo_plani_corto, NULL);
    pthread_detach(hilo_exitt);

    return;
}
// --------------------------- Pedidos memoria ---------------------------
int pedir_memoria(int socket)
{
    if (list_size(lista_procesos_new) > 0)
    {
            pthread_mutex_lock(&m_lista_procesos_new);
            pcb *proceso_nuevo = list_get(lista_procesos_new, 0);
            pthread_mutex_unlock(&m_lista_procesos_new);
            int pid = proceso_nuevo->pid;
            int tamanio = proceso_nuevo->tam_proc;
            char *path = proceso_nuevo->path_proc;
            int motivo = PROCESS_CREATE; // preguntar si solo es para PROCESS CREATE entonces mandarle un nombre mas descriptivo
            uint32_t size_path_hilo = strlen(path);


            t_paquete *pedido_memoria = crear_paquete(motivo);
            agregar_a_paquete_solo(pedido_memoria, &pid, sizeof(int));
            agregar_a_paquete_solo(pedido_memoria, &tamanio, sizeof(int));
            agregar_a_paquete_solo(pedido_memoria, &(size_path_hilo), sizeof(uint32_t));
            agregar_a_paquete_solo(pedido_memoria, path, size_path_hilo + 1);

            enviar_paquete(pedido_memoria, socket);
            eliminar_paquete(pedido_memoria);

            int confirmacion_mem_disponible = 0;
            // recv(socket,&confirmacion_mem_disponible, sizeof(int), MSG_WAITALL);

            int bytes_recibidos = recv(socket, &confirmacion_mem_disponible, sizeof(int), MSG_WAITALL);
            if (bytes_recibidos == -1)
            {
                perror("Error en recv");
                return;
            }
            else if (bytes_recibidos == 0)
            {
                printf("La conexión se cerró inesperadamente\n");
                return;
            }
            close(socket);
            sem_post(&bin_memoria);

            if (confirmacion_mem_disponible == 0)
            {
               // log_info(logger_kernel, "BORRAR: No hay memoria disponible para el proceso PID: %i", pid);
                //sem_post(&binario_corto_plazo);

                return 0;
            }
            else
            {
                pthread_mutex_lock(&m_lista_procesos_new);
                list_remove(lista_procesos_new, 0);
                pthread_mutex_unlock(&m_lista_procesos_new);

                tcb *hilo_main = crear_tcb(proceso_nuevo, proceso_nuevo->prioridad_hilo_main);
                agregar_a_ready_segun_alg(hilo_main);
                pthread_mutex_lock(&m_lista_procesos_new);
                if (list_size(lista_procesos_new) > 0)
                {   
                   // log_info(logger_kernel, "Alguna vez entras aca???");
                    pthread_mutex_unlock(&m_lista_procesos_new);
                    sem_wait(&bin_memoria);
                    int socket_recursivo = conectarMemoria();
                    pedir_memoria(socket_recursivo);
                } 
                else {pthread_mutex_unlock(&m_lista_procesos_new);
                
                }
            }
        } /*else {
            sem_post(&bin_memoria);
        }*/
}




//  --------------------------- Crear  ---------------------------

pcb *crear_pcb(int prioridad_h_main, char *path, int tamanio, int socket)
{
    pcb *nuevo_pcb = (pcb *)malloc(sizeof(pcb));

    nuevo_pcb->tam_proc = tamanio;
    nuevo_pcb->path_proc = (char *)malloc(strlen(path));
    strcpy(nuevo_pcb->path_proc, path);
    nuevo_pcb->pid = id_counter;
    nuevo_pcb->contador_tid = 0;
    nuevo_pcb->prioridad_hilo_main = prioridad_h_main;
    id_counter++;

    pthread_mutex_lock(&m_contador);
    contador++;
    pthread_mutex_unlock(&m_contador);
    nuevo_pcb->lista_tcb = list_create();
    nuevo_pcb->lista_mutex_proc = list_create();
    if (nuevo_pcb == NULL)
    {
        list_destroy(nuevo_pcb->lista_mutex_proc);
        list_destroy(nuevo_pcb->lista_tcb);
        free(nuevo_pcb);
        return NULL;
    }
    log_info(logger_kernel, "## (PID <%d> : TID <0>) Se crea el proceso - Estado: NEW", nuevo_pcb->pid);


    pthread_mutex_lock(&m_lista_procesos_new);
    list_add(lista_procesos_new, nuevo_pcb);
    pthread_mutex_unlock(&m_lista_procesos_new);
    pedir_memoria(socket);

 
}

tcb *crear_tcb(pcb *proc_padre, int prioridad)
{
    tcb *nuevo_tcb = (tcb *)malloc(sizeof(tcb));
    nuevo_tcb->tid = proc_padre->contador_tid;
    nuevo_tcb->pcb_padre_tcb = proc_padre;
    nuevo_tcb->prioridad = prioridad;
    nuevo_tcb->block_join = list_create();
    nuevo_tcb->lista_mutex = list_create();

    inicializar_registros(nuevo_tcb);
    proc_padre->contador_tid++;

    if (nuevo_tcb == NULL)
    {

        free(nuevo_tcb);
        return NULL;
    }
    list_add(proc_padre->lista_tcb, nuevo_tcb);
    log_info(logger_kernel, "## (PID <%d> : TID <%d>) Se crea el Hilo - Estado: READY",
             nuevo_tcb->pcb_padre_tcb->pid, nuevo_tcb->tid);

    return nuevo_tcb;
}

void crear_cola_nivel(int prioridad, tcb *hilo_c)
{
    nivel_prioridad *nuevo_nivel = (nivel_prioridad *)malloc(sizeof(nivel_prioridad));
    nuevo_nivel->prioridad = hilo_c->prioridad;
    nuevo_nivel->hilos_asociados = list_create();
    pthread_mutex_init(&(nuevo_nivel->m_lista_prioridad), NULL);

    pthread_mutex_lock(&(nuevo_nivel->m_lista_prioridad));
    list_add((nuevo_nivel->hilos_asociados), hilo_c);

    pthread_mutex_unlock(&(nuevo_nivel->m_lista_prioridad));

    pthread_mutex_lock(&m_lista_multinivel);
    list_add(lista_multinivel, nuevo_nivel);
    pthread_mutex_unlock(&m_lista_multinivel);
}

// -------------------------- Funciones planificador  ---------------------------
/*void inicializar_hilos_planificacion()
{
    pthread_t hilo_plani_corto,hilo_exitt;

    pthread_create(&hilo_plani_corto, NULL,(void*) planificador_corto_plazo,NULL);
    pthread_create(&hilo_exitt,NULL, (void*) hilo_exit, NULL);

    /*pthread_create(&hilo_plani_largo,NULL,(void*) planificador_largo_plazo,NULL);

    pthread_detach(hilo_plani_largo);*/
/*
pthread_detach(hilo_exitt);
pthread_detach(hilo_plani_corto);
}*/

int verificar_lista_ready(t_list *lista_de_ready)
{
    pthread_mutex_lock(&m_lista_de_ready);
    if (list_is_empty(lista_de_ready))
    {
        log_error(logger_kernel, "Cola de ready esta vacia en planificador corto plazo\n");
        pthread_mutex_unlock(&m_lista_de_ready);
        exit(1);
    }
    pthread_mutex_unlock(&m_lista_de_ready);
    return 1;
}

void mandar_tcb_dispatch(tcb *tcb_listo)
{
    sem_wait(&bin_dispatch);
    t_paquete *tcb_a_dispatch = crear_paquete(OP_ENVIO_TCB);
    agregar_a_paquete_solo(tcb_a_dispatch, &tcb_listo->tid, sizeof(int));
    agregar_a_paquete_solo(tcb_a_dispatch, &tcb_listo->pcb_padre_tcb->pid, sizeof(int));
    enviar_paquete(tcb_a_dispatch, conexion_dispatch);
    sem_post(&bin_dispatch);
    eliminar_paquete(tcb_a_dispatch);
    pthread_mutex_lock(&m_syscall_solicitada);
    syscall_solicitada = 0;
    pthread_mutex_unlock(&m_syscall_solicitada);
}

void desalojar_hilo(int motivo)
{
    t_paquete *paquete_a_desalojar = crear_paquete(DESALOJAR_PROCESO);
    agregar_a_paquete_solo(paquete_a_desalojar, &motivo, sizeof(int));
    agregar_a_paquete_solo(paquete_a_desalojar, &hilo_en_ejecucion->pcb_padre_tcb->pid, sizeof(int));
    agregar_a_paquete_solo(paquete_a_desalojar, &hilo_en_ejecucion->tid, sizeof(int));
    enviar_paquete(paquete_a_desalojar, conexion_interrupt);

    eliminar_paquete(paquete_a_desalojar);
}

double quantumf()
{
    double cuanto = atoi(config_get_string_value(config_kernel, "QUANTUM"));
    //  printf("El quantum es: %f\n", cuanto);
    return cuanto;
}

void *desalojar_por_RR(tcb *hilo)
{

    while (1)
    {
        usleep(quantumf() * 1000);
       
            pthread_mutex_lock(&m_syscall_replanificadora);
            pthread_mutex_lock(&m_hilo_en_ejecucion);
            if  ((hilo_en_ejecucion->tid == hilo->tid) 
                && (hilo_en_ejecucion->pcb_padre_tcb->pid == hilo->pcb_padre_tcb->pid)
                && syscall_replanificadora == 0)
            {
                pthread_mutex_unlock(&m_hilo_en_ejecucion);
                pthread_mutex_unlock(&m_syscall_replanificadora); 
                
                pthread_mutex_lock(&m_quantum_restante);
                quantum_restante = 0;
                pthread_mutex_unlock(&m_quantum_restante);
                
                log_info(logger_kernel," ## (PID <%d>:TID <%d>) - Desalojado por fin de Quantum", hilo->pcb_padre_tcb->pid, hilo->tid );
                desalojar_hilo(RR);
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&m_syscall_replanificadora);
            pthread_mutex_unlock(&m_hilo_en_ejecucion);
    }
}

void agregar_a_ready_segun_alg(tcb *hilo)
{
    if (strcmp(algoritmo_de_planificacion, "CMN") == 0)
    {
        agregar_a_ready_multinivel(hilo);
    }
    else
    {
        agregar_a_ready(hilo);
    }
}
// // -------------------------- Parametros  ---------------------------
void recibir_syscall_de_cpu(tcb *hilo, int *motivo, instruccion *instrucc)
{
    sem_wait(&bin_dispatch);
    int cod_op = recibir_operacion(conexion_dispatch);
    if (cod_op == SYSCALL)
    {
        sem_post(&bin_dispatch);
        desempaquetar_parametros_syscall_de_cpu(hilo, motivo, instrucc);
        //printf("TID: %i, motivo: %i\n", hilo->tid, *motivo);
    }
    else
    {
        sem_post(&bin_dispatch);
        printf("Codigo de Operacion de CPU incorrecto\n");
    }
}

void desempaquetar_parametros_syscall_de_cpu(tcb *hilo, int *motivo, instruccion *instrucc)
{
    sem_wait(&bin_dispatch);
    int tam;
    void *buffer = recibir_buffer_vieja(&tam, conexion_dispatch);
    int desplazamiento = 0;
    sem_post(&bin_dispatch);
    memcpy(motivo, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
   // printf("TID 2: %i, motivo 2: %i", hilo->tid, *motivo);
    memcpy(&(instrucc->cant_parametros), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    for (int i = 0; i < instrucc->cant_parametros; i++)
    {

        char *parametro;
        int tamanio_parametro;
        memcpy(&(tamanio_parametro), buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);

        parametro = (char *)calloc(1, tamanio_parametro);
        // parametro=malloc(size_parametro);
        if (parametro == NULL)
        {
            log_error(logger_kernel, "Error al asignar memoria para el parámetro");
            free(buffer);
            return;
        }
        memcpy(parametro, buffer + desplazamiento, tamanio_parametro);
        desplazamiento += tamanio_parametro;
        // printf("%d\n",i);
        list_add(instrucc->parametros, parametro);
    }

    free(buffer);
}

// --------------------- Iniciar hilos ---------------------
void iniciar_hilo(tcb *hilo, int conexion_memoria, char *path)
{

    // uint32_t size_path_hilo = sizeof(path);
    t_paquete *paquete = crear_paquete(INICIAR_HILO);
    agregar_a_paquete_solo(paquete, &(hilo->pcb_padre_tcb->pid), sizeof(uint32_t));
    agregar_a_paquete_solo(paquete, &(hilo->tid), sizeof(uint32_t));
    // agregar_a_paquete_solo(paquete, &(size_path_hilo), sizeof(uint32_t));
    agregar_a_paquete(paquete, path, strlen(path) + 1);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    int confirmacion;
    recv(conexion_memoria, &confirmacion, sizeof(int), MSG_WAITALL);

    if (confirmacion == 1)
    {
        // agregue este sem post xq si no nunca podia pasar el semwait de agregar a nuevos!!
        if (hilo == NULL)
        {
            printf("HILO trucho1");
        }
        agregar_a_ready_segun_alg(hilo);
    }
}

// --------------------- Finalizar ---------------------
void finalizar_proceso(pcb *proc)
{
    avisar_memoria_liberar_pcb(proc);

    log_info(logger_kernel, "## Finaliza el proceso <%d>", proc->pid);
    if (list_size(lista_procesos_new) > 0){
        log_info(logger_kernel, "Finalizar proc: tamaño lista proc new %d", list_size(lista_procesos_new));
        sem_wait(&bin_memoria);
        int socketFP = conectarMemoria();
        pedir_memoria(socketFP);
        //close(socketFP);
    }
   

    if (!list_is_empty(proc->lista_tcb)) // hago este if xq tmbn llamo a esta funcion cuando se queda sin hilos
    {
        for (int i = 0; i < list_size(proc->lista_tcb); i++)
        {
            tcb *hilo_a_finalizar = list_remove(proc->lista_tcb, 0);
            buscar_hilos_listas(hilo_a_finalizar, hilo_a_finalizar->tid);
            pthread_mutex_lock(&m_lista_finalizados);
            list_add(lista_finalizados, hilo_a_finalizar);
            pthread_mutex_unlock(&m_lista_finalizados);
            liberar_tcb(hilo_a_finalizar);
            sem_post(&hilos_en_exit);
        }
    }

    list_destroy(proc->lista_tcb);
    
    void mutex_proc_destroy(void *ptr)
    {
        mutex_k *mutex = (mutex_k *)ptr;
        list_destroy(mutex->bloqueados_por_mutex);
        free(mutex);
    }
    // pthread_mutex_lock(&m_lista_mutex);
    list_destroy_and_destroy_elements(proc->lista_mutex_proc, mutex_proc_destroy);
    // pthread_mutex_unlock(&m_lista_mutex);
    pthread_mutex_lock(&m_contador);
    contador--;
    pthread_mutex_unlock(&m_contador);
    free(proc->path_proc);
    free(proc);
    

    sem_post(&finalizo_un_proc);

    sem_post(&binario_corto_plazo);
}

void finalizar_hilos_proceso(pcb *proceso) // PARA PROCESS_EXIT
{

    tcb *hilo_a_finalizar;
    while (list_is_empty(proceso->lista_tcb) != true)
    {
        hilo_a_finalizar = list_get(proceso->lista_tcb, 0);
        finalizar_tcb(hilo_a_finalizar);
    }
}

void finalizar_tcb(tcb *hilo_a_finalizar){
    buscar_hilos_listas(hilo_a_finalizar, hilo_a_finalizar->tid);
    pthread_mutex_lock(&m_lista_finalizados);
    list_add(lista_finalizados, hilo_a_finalizar);
    pthread_mutex_unlock(&m_lista_finalizados);

    avisar_memoria_liberar_tcb(hilo_a_finalizar);
    log_info(logger_kernel, "## (PID <%d>:TID <%d>) Finaliza el hilo", hilo_a_finalizar->pcb_padre_tcb->pid, hilo_a_finalizar->tid);
    liberar_tcb(hilo_a_finalizar);
    sem_post(&hilos_en_exit);
    

}

void *hilo_exit()
{
    while (1)
    {
        sem_wait(&hilos_en_exit);

        pthread_mutex_lock(&m_lista_finalizados);
        tcb *hilo = list_remove(lista_finalizados, 0);
        pthread_mutex_unlock(&m_lista_finalizados);
        free(hilo);
        // sem_post(&binario_corto_plazo);
    }
}

void liberar_tcb(tcb *hilo)
{
    liberar_mutexs_asociados(hilo);
    liberar_bloqueados_x_thread_join(hilo);
}

void finalizar_estructuras_kernel()
{
    // TO DO ;/

    // listas
    list_destroy(lista_multinivel);
    list_destroy(lista_de_ready);
    list_destroy(lista_procesos_new);
    list_destroy(lista_finalizados);
    list_destroy(lista_io);
    list_destroy(bloqueados_por_dump);

    // mutex
    pthread_mutex_destroy(&m_hilo_en_ejecucion);
    pthread_mutex_destroy(&m_lista_de_ready);
    pthread_mutex_destroy(&m_regreso_de_cpu);
    pthread_mutex_destroy(&m_hilo_a_ejecutar);
    pthread_mutex_destroy(&m_lista_procesos_new);
    pthread_mutex_destroy(&m_lista_multinivel);
    pthread_mutex_destroy(&m_lista_finalizados);
    pthread_mutex_destroy(&m_quantum_restante);
    pthread_mutex_destroy(&m_lista_io);
    pthread_mutex_destroy(&m_bloqueados_por_dump);
    pthread_mutex_destroy(&m_contador);
    pthread_mutex_destroy(&m_syscall_replanificadora);
    pthread_mutex_destroy(&m_lista_mutex);

    // semaforos
    sem_destroy(&finalizo_un_proc);
    sem_destroy(&hilos_en_exit);
    sem_destroy(&hilos_en_ready);
    sem_destroy(&binario_corto_plazo);
    sem_destroy(&bin_memoria);
    sem_destroy(&bin_dispatch);
}
// --------------------- Buscar ---------------------
tcb *buscar_TID(tcb *tcb_pedido, int tid_buscado)
{
    pcb *proc = tcb_pedido->pcb_padre_tcb;
    for (int i = 0; i < list_size(proc->lista_tcb); i++)
    {
        tcb *hilo_buscado = list_get(proc->lista_tcb, i);
        if (hilo_buscado->tid == tid_buscado)
        {
            list_remove(proc->lista_tcb, i); // Buscar y lo saco del pcb
            return hilo_buscado;
        }
    }

    return NULL;
}

tcb *buscar_hilos_listas(tcb *main, int tid)
{
    tcb *hilo = buscar_TID(main, tid); // este solo lo saca de la lista del padre
    int confirmacion = 0;
    if (hilo != NULL)
    {
        if (strcmp(algoritmo_de_planificacion, "CMN") == 0)
        {
           // printf("pid %d tid %d\n", hilo->pcb_padre_tcb->pid, tid);
            hilo = buscar_hilo_en_multinivel(main->prioridad, main->tid, main->pcb_padre_tcb);

            if (hilo)
            {
                return hilo;
            }
        }
        else
        {
            pthread_mutex_lock(&m_lista_de_ready);
            confirmacion = list_remove_element(lista_de_ready, hilo);
            pthread_mutex_unlock(&m_lista_de_ready);
            if (confirmacion)
            {
                return hilo;
            }
        }
        return hilo;
    }

    return NULL;
}

tcb *buscar_hilos_listas_sin_sacar_del_padre(tcb *main, int tid)
{
    tcb *hilo = buscar_tid(main->pcb_padre_tcb->lista_tcb, tid); // este no lo saca de la lista del padre
    int confirmacion = 0;
    if (hilo != NULL)
    {
        if (strcmp(algoritmo_de_planificacion, "CMN") == 0)
        {
           // printf("pid %d tid %d\n", hilo->pcb_padre_tcb->pid, tid);
            tcb *hilo_multinivel = buscar_hilo_en_multinivel(main->prioridad, main->tid, main->pcb_padre_tcb);

            if (hilo_multinivel != NULL)
            {
                return hilo_multinivel;
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            pthread_mutex_lock(&m_lista_de_ready);
            confirmacion = list_remove_element(lista_de_ready, hilo);
            pthread_mutex_unlock(&m_lista_de_ready);
            if (confirmacion)
            {
                return hilo;
            }
        }
        return hilo;
    }

    return NULL;
}

tcb *buscar_tid(t_list *lista_tcb, int tid)
{

    for (int i = 0; i < list_size(lista_tcb); i++)
    {
        tcb *aux = list_get(lista_tcb, i);
        if (aux->tid == tid)
        {
            return aux;
        }
    }
    return NULL;
}

tcb *buscar_hilo_en_multinivel(int prioridad, int tid, pcb *padre)
{
    for (int i = 0; i < list_size(lista_multinivel); i++)
    {
        pthread_mutex_lock(&m_lista_multinivel);
        nivel_prioridad *cola_aux = list_get(lista_multinivel, i);
        pthread_mutex_unlock(&m_lista_multinivel);
        if (cola_aux->prioridad == prioridad)
        {

            pthread_mutex_lock(&(cola_aux->m_lista_prioridad));
            int cant_elementos = list_size(cola_aux->hilos_asociados);
            // printf("Cantidad de elementos %d\n", cant_elementos);
            for (int j = 0; j < cant_elementos; j++)
            {
                tcb *hilo = list_get(cola_aux->hilos_asociados, j);

                if (hilo->pcb_padre_tcb->pid == padre->pid){
                    if (hilo->tid == tid){
                        list_remove(cola_aux->hilos_asociados, j);
                        if (list_size(cola_aux->hilos_asociados) == 0) // VER
                        {
                            list_remove_element(lista_multinivel, cola_aux);
                            free(cola_aux);
                        }
                        pthread_mutex_unlock(&(cola_aux->m_lista_prioridad));
                        // printf("Aca lo encontro TID: %d Prioridad: %d \n", hilo->tid, hilo->prioridad);
                        return hilo;
                    }
                }
                pthread_mutex_unlock(&(cola_aux->m_lista_prioridad));
            }
        }
    }
    return NULL;
}

pcb *buscar_proc_lista(t_list *lista, int pid_buscado)
{
    int elementos = list_size(lista);
    for (int i = 0; i < elementos; i++)
    {
        pcb *pcb = list_get(lista, i);
        if (pid_buscado == pcb->pid)
        {
            return pcb;
        }
    }
    return NULL;
}

bool existe_mutex(mutex_k *mutex_solic, t_list *lista_mutexs_proceso)
{
    bool existe = false;
    mutex_k *aux;
    for (int i = 0; i < list_size(lista_mutexs_proceso); i++)
    {
        pthread_mutex_lock(&m_lista_mutex);
        aux = list_get(lista_mutexs_proceso, i);
        pthread_mutex_lock(&m_lista_mutex);
        if (strcmp(aux->nombre, mutex_solic->nombre) == 0)
        {
            existe = true;
            return existe;
        }
    }
    return existe;
}

mutex_k *existe_mutex_por_nombre(char *mutex_solic, t_list *lista_mutexs_proceso)
{
    // bool existe = false;
    mutex_k *aux;
    for (int i = 0; i < list_size(lista_mutexs_proceso); i++)
    {
        pthread_mutex_lock(&m_lista_mutex);
        aux = list_get(lista_mutexs_proceso, i);
        pthread_mutex_unlock(&m_lista_mutex);
        if (strcmp(aux->nombre, mutex_solic) == 0)
        {
            return aux;
        }
    }
    return NULL;
}

bool mutex_tomado_por_hilo(mutex_k *mutex, tcb *hilo)
{
    bool tomado = false;
    mutex_k *aux;
    for (int i = 0; i < list_size(hilo->lista_mutex); i++)
    {
        pthread_mutex_lock(&m_lista_mutex);
        aux = list_get(hilo->lista_mutex, i);
        pthread_mutex_unlock(&m_lista_mutex);
        if (strcmp(mutex->nombre, aux->nombre) == 0)
        {
            tomado = true;
            return tomado;
        }
    }

    return tomado;
}

bool mutex_por_nombre_tomado_por_hilo(char *mutex, tcb *hilo)
{
    bool tomado = false;
    mutex_k *aux;
    for (int i = 0; i < list_size(hilo->lista_mutex); i++)
    {
        pthread_mutex_lock(&m_lista_mutex);
        aux = list_get(hilo->lista_mutex, i);
        pthread_mutex_unlock(&m_lista_mutex);
        if (strcmp(mutex, aux->nombre) == 0)
        {
            tomado = true;
            return tomado;
        }
    }

    return tomado;
}
/*
t_list* encontrar_por_nivel(t_list* lista_multinivel, int prioridad)
{
    nivel_prioridad* aux;
    bool _existe_nivel
    {
        for(int i = 0; i < list_size(lista_multinivel); i++)
        {
            aux = list_get(lista_multinivel, i);
            if(aux->prioridad == prioridad)
            {
                return aux->hilos_asociados;
            }
        }
    }
    return list_find(lista_multinivel, _existe_nivel);
}*/

nivel_prioridad *encontrar_por_nivel(t_list *lista_multi, int prioridad)
{
    int cant_elem_multi = list_size(lista_multi);
    // printf("Cantidad de elementos lista multinivel %d \n", cant_elem_multi);
    for (int i = 0; i < cant_elem_multi; i++)
    {
        nivel_prioridad *resultado = list_get(lista_multi, i);
        if (resultado->prioridad == prioridad)
        {
           // printf("Existe el nivel de prioridad %d \n", resultado->prioridad);
            return resultado;
        }
    }
    return NULL;
}

nivel_prioridad *encontrar_nivel_mas_prioritario(t_list *multinivel)
{
    nivel_prioridad *mayor_prioridad;

    void *_max_prioridad(void *a, void *b)
    {
        nivel_prioridad *nivel_a = (nivel_prioridad *)a;
        nivel_prioridad *nivel_b = (nivel_prioridad *)b;

        return nivel_a->prioridad <= nivel_b->prioridad ? nivel_a : nivel_b;
    }
    if (list_size(multinivel) > 0)
    {
        mayor_prioridad = list_get_maximum(multinivel, _max_prioridad);
        return mayor_prioridad;
    }
    return NULL;
}

// --------------------- Dump ---------------------

int bloquear_por_dump(tcb *hilo, int socket)
{
    int finalizo_operacion = 0;
    t_paquete *dump = crear_paquete(DUMP_MEMORY);
    agregar_a_paquete_solo(dump, &(hilo->pcb_padre_tcb->pid), sizeof(int));
    agregar_a_paquete_solo(dump, &hilo->tid, sizeof(int));
    enviar_paquete(dump, socket);
    eliminar_paquete(dump);

    
    pthread_mutex_lock(&m_bloqueados_por_dump);
    list_add(bloqueados_por_dump, hilo);
    pthread_mutex_unlock(&m_bloqueados_por_dump);
    //printf("TAmanio bloqueado por dump %d \n", list_size(bloqueados_por_dump));
    

    return 0;
}
// --------------------- Mutex ---------------------
mutex_k *crear_mutex(char *nombre)
{
    mutex_k *mtx = (mutex_k *)malloc(sizeof(mutex_k));
    mtx->nombre = (char *)malloc(strlen(nombre));
    strcpy(mtx->nombre, nombre);
    mtx->disponibilidad = true;
    mtx->bloqueados_por_mutex = list_create();

    return mtx;
}
void asignar_mutex_hilo(mutex_k *mutex, tcb *hilo)
{
    mutex->disponibilidad = false;
    mutex->hilo_poseedor = hilo;
    pthread_mutex_lock(&m_lista_mutex);
    list_add(hilo->lista_mutex, mutex);
    pthread_mutex_unlock(&m_lista_mutex);
    log_info(logger_kernel, "## Se asigna el MUTEX: <%s> a PID <%d> : TID <%d>",
             mutex->nombre, hilo->pcb_padre_tcb->pid, hilo->tid);
}

void liberar_mutexs_asociados(tcb *hilo)
{
    mutex_k *aux;
    pthread_mutex_lock(&m_lista_mutex);
    int elementos = list_size(hilo->lista_mutex);
    pthread_mutex_unlock(&m_lista_mutex);
    for (int i = 0; i < elementos; i++)
    {
        pthread_mutex_lock(&m_lista_mutex);
        aux = list_remove(hilo->lista_mutex, i);
        pthread_mutex_unlock(&m_lista_mutex);
        asignar_mutex_al_primer_bloqueado(aux);
    }
    list_destroy(hilo->lista_mutex);
}

void asignar_mutex_al_primer_bloqueado(mutex_k *mutex_solicitado)
{
    tcb *bloq_por_mutex;
    if (list_size(mutex_solicitado->bloqueados_por_mutex) > 1)
    {
        bloq_por_mutex = list_remove(mutex_solicitado->bloqueados_por_mutex, 0);
        asignar_mutex_hilo(mutex_solicitado, bloq_por_mutex);
        agregar_a_ready_segun_alg(bloq_por_mutex);
    }
    else if (list_size(mutex_solicitado->bloqueados_por_mutex) == 1)
    {
        bloq_por_mutex = list_remove(mutex_solicitado->bloqueados_por_mutex, 0);
        asignar_mutex_hilo(mutex_solicitado, bloq_por_mutex);

        mutex_solicitado->disponibilidad = true;
        mutex_solicitado->hilo_poseedor = NULL;
        agregar_a_ready_segun_alg(bloq_por_mutex);
    }
}

// --------------------- funciones auxiliares ---------------------
void liberar_param_instruccion(instruccion *instrucc)
{
    for (int i = 0; i < list_size(instrucc->parametros); i++)
    {
        free(list_get(instrucc->parametros, i));
    }
    list_destroy(instrucc->parametros);
    free(instrucc);
}
/*
void sacar_de_lista_pcb(tcb* hilo_a_sacar)
{
    pcb* proc_padre = hilo_a_sacar->pcb_padre_tcb;
    bool lo_encontro;
    lo_encontro = list_remove_element(proc_padre->lista_tcb, hilo_a_sacar);

    if(lo_encontro != 0)
    {
        //proc_padre->contador_tid--;
        if(proc_padre->contador_tid <= 0)
        {
            finalizar_proceso(proc_padre);
        }
    }

}*/

void liberar_bloqueados_x_thread_join(tcb *hilo)
{
    tcb *aux;
    if (hilo->block_join != NULL && list_size(hilo->block_join) > 0)
    {
        for (int i = 0; i < list_size(hilo->block_join); i++)
        {
            aux = list_remove(hilo->block_join, i);
            agregar_a_ready_segun_alg(aux);
        }
    }
    list_destroy(hilo->block_join);
}

void avisar_memoria_liberar_tcb(tcb *hilo)
{
    sem_wait(&bin_memoria);
    int socket = conectarMemoria();
    t_paquete *t_exit = crear_paquete(THREAD_EXIT);
    agregar_a_paquete_solo(t_exit, &(hilo->pcb_padre_tcb->pid), sizeof(int));
    agregar_a_paquete_solo(t_exit, &hilo->tid, sizeof(int));
    enviar_paquete(t_exit, socket);
    eliminar_paquete(t_exit);
    int confirmacion;
    recv(socket, &confirmacion, sizeof(int), MSG_WAITALL);
    close(socket);
    sem_post(&bin_memoria);
}

void avisar_memoria_liberar_pcb(pcb *proc)
{
    sem_wait(&bin_memoria);
    int socket = conectarMemoria();
    t_paquete *p_exit = crear_paquete(PROCESS_EXIT);
    agregar_a_paquete_solo(p_exit, &proc->pid, sizeof(int));
    enviar_paquete(p_exit, socket);
    eliminar_paquete(p_exit);
    int confirmacion;
    recv(socket, &confirmacion, sizeof(int), MSG_WAITALL);
   // printf("CONFIRMO FINALIZO PROC EN MEM: %d \n", confirmacion);
    close(socket);
    sem_post(&bin_memoria);
}
