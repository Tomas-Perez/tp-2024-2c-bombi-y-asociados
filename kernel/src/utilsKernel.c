// quizas no esta en la mejor carpeta
/*#include <stdio.h>
#include<stdlib.h>*/

#include "utilsKernel.h"

t_config* config_kernel;
t_log* logger_kernel;
char *ip_memoria;
char *puerto_memoria; 
char *ip_cpu; 
char *puerto_cpu_dispatch; 
char *puerto_cpu_interrupt; 
char *algoritmo_de_planificacion;  
int quantum; 
int syscall_solicitada;
char *log_level;

//---------------------------------------------------------------- 
// --------------------------- inidice --------------------------- 
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


char* generar_path_archivo(char* nombre_archivo)
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
    //config_kernel = iniciar_config("kernelFS.config");
    //config_kernel = iniciar_config("kernelRC.config");
    //config_kernel = iniciar_config("kernelParticionesDinamicas.config");
    //config_kernel = iniciar_config("kernelParticionesFijas.config");

    config_kernel = iniciar_config("kernelPlani.config");
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config_kernel, "IP_CPU");    
    puerto_cpu_dispatch = config_get_string_value(config_kernel, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config_kernel, "PUERTO_CPU_INTERRUPT");
    algoritmo_de_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    quantum = config_get_int_value(config_kernel, "QUANTUM");
    log_level = config_get_string_value(config_kernel, "LOG_LEVEL");

}



void inicializar_registros(tcb* hilo)
{
        hilo->registros_cpu.AX=0;
	    hilo->registros_cpu.BX=0;
	    hilo->registros_cpu.CX=0;
	    hilo->registros_cpu.DX=0;
	    hilo->registros_cpu.EX=0;
	    hilo->registros_cpu.FX=0;
	    hilo->registros_cpu.GX=0;
	    hilo->registros_cpu.HX=0;
    	hilo->registros_cpu.PC=0;

}

void inicializar_estructuras_kernel()
{
	int id_counter = 1;
    
    syscall_solicitada = 0;
	//mutex 
	pthread_mutex_init(&m_hilo_en_ejecucion,NULL);
	pthread_mutex_init(&m_lista_de_ready,NULL);
   // pthread_mutex_init(&m_lista_bloqueados,NULL);
	pthread_mutex_init(&m_regreso_de_cpu,NULL);
    pthread_mutex_init(&m_hilo_a_ejecutar, NULL); //TO DO destroy de los mutex
    pthread_mutex_init(&m_lista_procesos_new, NULL);
    pthread_mutex_init(&m_lista_multinivel,NULL);
    pthread_mutex_init(&m_lista_finalizados,NULL);
    pthread_mutex_init(&m_lista_prioridad,NULL);
    //semaforo
    sem_init(&finalizo_un_proc, 0, 0);
    sem_init(&hilos_en_exit, 0, 0);
    sem_init(&hilos_en_ready,0,0);
    sem_init(&binario_corto_plazo,0,1);
	 //cola de procesos
	lista_de_ready = list_create();
    lista_procesos_new = list_create();
    lista_multinivel = list_create();
    lista_finalizados = list_create();
     
}


void inicializar_hilos_planificacion()
{ 
    pthread_t hilo_plani_corto,hilo_exitt;

	pthread_create(&hilo_plani_corto, NULL,(void*) planificador_corto_plazo,NULL);
    pthread_create(&hilo_exitt,NULL, (void*) hilo_exit, NULL);

	/*pthread_create(&hilo_plani_largo,NULL,(void*) planificador_largo_plazo,NULL);
	
	pthread_detach(hilo_plani_largo);*/
	pthread_join(hilo_exitt,NULL); 
    pthread_join(hilo_plani_corto,NULL);
}
// --------------------------- Pedidos memoria ---------------------------
void pedir_memoria(int socket)
{ 
   
        pthread_mutex_lock(&m_lista_procesos_new);
       pcb* proceso_nuevo = list_get(lista_procesos_new, 0);
        pthread_mutex_unlock(&m_lista_procesos_new);
        int pid = proceso_nuevo->pid;
        int tamanio = proceso_nuevo->tam_proc;
        char* path = proceso_nuevo->path_proc;
        int motivo = PROCESS_CREATE; //preguntar si solo es para PROCESS CREATE entonces mandarle un nombre mas descriptivo
        uint32_t size_path_hilo = strlen(path);
        
        printf("2 Tamanio Path: %i\n", size_path_hilo);
        printf("2 Path: %s\n", path);

        t_paquete* pedido_memoria = crear_paquete(motivo);
		agregar_a_paquete_solo(pedido_memoria, &pid,sizeof(int));
        agregar_a_paquete_solo(pedido_memoria, &tamanio, sizeof(int));
        agregar_a_paquete_solo(pedido_memoria, &(size_path_hilo), sizeof(uint32_t));
        agregar_a_paquete_solo(pedido_memoria, path, size_path_hilo + 1);

        
        enviar_paquete(pedido_memoria,socket);
		eliminar_paquete(pedido_memoria);

        int confirmacion_mem_disponible = 0;
        //recv(socket,&confirmacion_mem_disponible, sizeof(int), MSG_WAITALL);

        int bytes_recibidos = recv(socket, &confirmacion_mem_disponible, sizeof(int), MSG_WAITALL);
    if (bytes_recibidos == -1) {
        perror("Error en recv");
        return;
    } else if (bytes_recibidos == 0) {
        printf("La conexión se cerró inesperadamente\n");
        return; 
    }   


        if(!confirmacion_mem_disponible)
        {
            sem_wait(&finalizo_un_proc);
            pedir_memoria(socket);	
        }
        else {
                pthread_mutex_lock(&m_lista_procesos_new);
                list_remove(lista_procesos_new, 0);
                pthread_mutex_unlock(&m_lista_procesos_new);
        }
        close(socket);
}


//  --------------------------- Crear  --------------------------- 

pcb *crear_pcb(int prioridad_h_main, char* path, int tamanio, int socket)
{
    pcb* nuevo_pcb = (pcb *)malloc(sizeof(pcb));
    tcb* hilo_main;

    nuevo_pcb->tam_proc = tamanio;
    nuevo_pcb->path_proc = path;
    nuevo_pcb->pid = id_counter;
    nuevo_pcb->contador_tid = 0;
    id_counter++;


    nuevo_pcb->lista_tcb = list_create();
    nuevo_pcb->lista_mutex_proc = list_create();
    if (nuevo_pcb == NULL)
    {
        list_destroy(nuevo_pcb->lista_mutex_proc);
        list_destroy(nuevo_pcb->lista_tcb);
        free(nuevo_pcb);
        return NULL;
    }
    log_info(logger_kernel, "## (<PID>:%d) Se crea el proceso - Estado: NEW", nuevo_pcb->pid);
    hilo_main = crear_tcb(nuevo_pcb, prioridad_h_main);

    pthread_mutex_lock(&m_lista_procesos_new);
    list_add(lista_procesos_new, nuevo_pcb);
    pthread_mutex_unlock(&m_lista_procesos_new);
    pedir_memoria(socket);
    
    hilo_main = list_get(nuevo_pcb->lista_tcb, 0);       
	//iniciar_hilo(hilo_main, socket, nuevo_pcb->path_proc);
    agregar_a_ready_segun_alg(hilo_main);
    //sem_post(&binario_corto_plazo);
    return nuevo_pcb;
}

tcb* crear_tcb(pcb* proc_padre, int prioridad)
{
    tcb* nuevo_tcb = (tcb *)malloc(sizeof(tcb));  
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

void crear_cola_nivel(int prioridad, tcb* hilo, nivel_prioridad* nuevo_nivel) 
{

    nuevo_nivel->prioridad = prioridad;
    nuevo_nivel->hilos_asociados = list_create();
    pthread_mutex_init(&nuevo_nivel->m_lista_prioridad, NULL);

    pthread_mutex_lock(&nuevo_nivel->m_lista_prioridad);
    list_add(nuevo_nivel->hilos_asociados, hilo);
    pthread_mutex_unlock(&nuevo_nivel->m_lista_prioridad);

    pthread_mutex_lock(&m_lista_multinivel);
    list_add(lista_multinivel, nuevo_nivel);
    pthread_mutex_unlock(&m_lista_multinivel);
    // VER: nos habian dicho q usemos list_add_sorted pero como
    // cuando lo pasamos a running encontramos el mayor nivel de prioridad
    // creo q no hace falta

}


// -------------------------- Funciones planificador  --------------------------- 
/*void inicializar_hilos_planificacion()
{ 
    pthread_t hilo_plani_corto,hilo_exitt;

	pthread_create(&hilo_plani_corto, NULL,(void*) planificador_corto_plazo,NULL);
    pthread_create(&hilo_exitt,NULL, (void*) hilo_exit, NULL);

	/*pthread_create(&hilo_plani_largo,NULL,(void*) planificador_largo_plazo,NULL);
	
	pthread_detach(hilo_plani_largo);*//*
	pthread_detach(hilo_exitt); 
    pthread_detach(hilo_plani_corto);
}*/

int verificar_lista_ready(t_list* lista_de_ready) {
    pthread_mutex_lock(&m_lista_de_ready);
    if (list_is_empty(lista_de_ready)) 
    {
    	log_error(logger_kernel, "Cola de ready esta vacia en planificador corto plazo\n");
    	pthread_mutex_unlock(&m_lista_de_ready);
		exit (1);
	}
      pthread_mutex_unlock(&m_lista_de_ready);   
      return 1;
}

void mandar_tcb_dispatch(tcb* tcb_listo)
{
	t_paquete* tcb_a_dispatch = crear_paquete(OP_ENVIO_TCB);
    agregar_a_paquete_solo(tcb_a_dispatch,&tcb_listo->tid, sizeof(int));
    agregar_a_paquete_solo(tcb_a_dispatch,&tcb_listo->pcb_padre_tcb->pid, sizeof(int));
	enviar_paquete(tcb_a_dispatch,conexion_dispatch);
    eliminar_paquete(tcb_a_dispatch);
}


void desalojar_hilo(int motivo)
{
    t_paquete *paquete_a_desalojar = crear_paquete(DESALOJAR_PROCESO);
	//agregar_a_paquete_solo(paquete_a_desalojar,&motivo, sizeof(int));
	agregar_a_paquete_solo(paquete_a_desalojar,&hilo_en_ejecucion->tid, sizeof(int));
	enviar_paquete(paquete_a_desalojar, conexion_interrupt);
	eliminar_paquete(paquete_a_desalojar);
}

void* desalojar_por_RR(tcb* hilo)
{
    usleep(quantum*1000);
    if((hilo_en_ejecucion->tid == hilo->tid) && syscall_solicitada == 0)
	{		
		desalojar_hilo(RR);
		log_info(logger_kernel,"## (PID <%d>:TID <%d>) - Desalojado por fin de Quantum”",hilo->pcb_padre_tcb->pid,hilo->tid);
		//return;
	}
}

void agregar_a_ready_segun_alg(tcb* hilo)
{
	if(strcmp(algoritmo_de_planificacion, "CMN") == 0)
	{
		agregar_a_ready_multinivel(hilo);
	}
	else
	{
		agregar_a_ready(hilo);
	}
}
// // -------------------------- Parametros  --------------------------- 
void recibir_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc){
		int cod_op = recibir_operacion(conexion_dispatch);
		if(cod_op == SYSCALL){
            desempaquetar_parametros_syscall_de_cpu(hilo, motivo, instrucc);
			printf("TID: %i, motivo: %i\n", hilo->tid, *motivo);
            
		}
		else {
            printf("Codigo de Operacion de CPU incorrecto\n");
        }
       //desempaquetar_parametros_syscall_de_cpu(hilo, motivo, instrucc);
}

void desempaquetar_parametros_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc){
		int tam;
		void* buffer = recibir_buffer_vieja(&tam, conexion_dispatch);
		int desplazamiento = 0;
		
		memcpy(motivo, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		//printf("TID 2: %i, motivo 2: %i", hilo->tid, *motivo);
		memcpy(&(instrucc->cant_parametros), buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		for(int i = 0; i < instrucc->cant_parametros; i++){

			char* parametro; 
			int tamanio_parametro;
			memcpy(&(tamanio_parametro), buffer + desplazamiento, sizeof(int));
			desplazamiento += sizeof(int);
            
			parametro = (char *) calloc(1,tamanio_parametro);
			//parametro=malloc(size_parametro);
			if (parametro == NULL) 
            {
                // Manejar el error de memoria
                log_error(logger_kernel, "Error al asignar memoria para el parámetro");
                // Liberar recursos y salir de la función
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
void iniciar_hilo(tcb* hilo, int conexion_memoria, char* path){
        
        uint32_t size_path_hilo = sizeof(path);
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
    if(!list_is_empty(proc->lista_tcb))  // hago este if xq tmbn llamo a esta funcion cuando se queda sin hilos
    {
        finalizar_hilos_proceso(proc);
    }
    avisar_memoria_liberar_pcb(proc);
    log_info(logger_kernel,"## Finaliza el proceso <%d>",proc->pid);

    list_destroy(proc->lista_tcb);
    free(proc->path_proc); 
    free(proc);
    sem_post(&finalizo_un_proc);
}


void finalizar_hilos_proceso(pcb* proceso)  // PARA PROCESS_EXIT
{
   
    tcb* hilo_a_finalizar;
    while(list_is_empty(proceso->lista_tcb) == 0)
    {
        hilo_a_finalizar = list_get(proceso->lista_tcb,0);
        finalizar_tcb(hilo_a_finalizar);
    }
}

void finalizar_tcb(tcb* hilo_a_finalizar)
{
    buscar_hilos_listas(hilo_a_finalizar, hilo_a_finalizar->tid);
    pthread_mutex_lock(&m_lista_finalizados);
    list_add(lista_finalizados, hilo_a_finalizar); 
    pthread_mutex_unlock(&m_lista_finalizados);

    sem_post(&hilos_en_exit);
    log_info(logger_kernel,"## (PID <%d>:TID <%d>) Finaliza el hilo", hilo_a_finalizar->pcb_padre_tcb->pid,hilo_a_finalizar->tid);
}

void* hilo_exit()
{
	while(1)
	{
		sem_wait(&hilos_en_exit);
	
		pthread_mutex_lock(&m_lista_finalizados);
		tcb *hilo = list_remove(lista_finalizados,0);
		pthread_mutex_unlock(&m_lista_finalizados);

        avisar_memoria_liberar_tcb(hilo);
		
		liberar_tcb(hilo);
        sem_post(&binario_corto_plazo);
		printf("BORRAR: en hilo_exit-> termino todo\n");
	}
	
}

void liberar_tcb(tcb* hilo)
{
    liberar_mutexs_asociados(hilo);
	liberar_bloqueados_x_thread_join(hilo);
    free(hilo);
}




void finalizar_estructuras_kernel()
{
    // TO DO ;/

    // listas
    list_destroy(lista_multinivel);
    list_destroy(lista_de_ready);
    list_destroy(lista_procesos_new);
    //list_destroy();
    

}
// --------------------- Buscar ---------------------
tcb* buscar_TID(tcb* tcb_pedido, int tid_buscado){
	pcb* proc = tcb_pedido->pcb_padre_tcb;
	for (int i = 0; i < list_size(proc->lista_tcb); i++) {
	tcb* hilo_buscado = list_get(proc->lista_tcb, i);
        if (hilo_buscado->tid == tid_buscado) {
			list_remove(proc->lista_tcb, i);//Buscar y lo saco del pcb
            return hilo_buscado;
			
        }
    }
    
    return NULL;
}


tcb* buscar_hilos_listas(tcb* main, int tid){
	tcb* hilo = buscar_TID(main, tid);
	int confirmacion = 0;
	if (hilo != NULL) {
        if(strcmp(algoritmo_de_planificacion,"CMN") == 0){
            printf("tid %d\n", tid);
			hilo = buscar_hilo_en_multinivel(hilo->prioridad, hilo->tid);
            
            if (hilo) {
                 return hilo;  
             }
		} 
        else {
            pthread_mutex_lock(&m_lista_de_ready);
            confirmacion = list_remove_element(lista_de_ready, hilo);
		    pthread_mutex_unlock(&m_lista_de_ready);
            if (confirmacion) {
                 return hilo;  
             }
        
        }
    }

    return NULL;
	}


tcb* buscar_hilo_en_multinivel(int prioridad, int tid) {
    printf("Tid %d Prioridad %d\n", tid, prioridad);
    for (int i = 0; i < list_size(lista_multinivel); i++) {
        nivel_prioridad* cola_aux = list_get(lista_multinivel, i);
        
        if (cola_aux->prioridad == prioridad) {
           
            pthread_mutex_lock(&(cola_aux->m_lista_prioridad));
            int cant_elementos = list_size(cola_aux->hilos_asociados);
            printf("Cantidad de elementos %d\n", cant_elementos);
            for (int j = 0; j < cant_elementos; j++) {
                tcb* hilo = list_get(cola_aux->hilos_asociados, j);
                if (hilo->tid == tid) {
                   	list_remove(cola_aux->hilos_asociados, j);
                    if(list_size(cola_aux->hilos_asociados) == 0) //VER
                    {
                        free(cola_aux);
                    }
                    pthread_mutex_unlock(&(cola_aux->m_lista_prioridad));
                    printf("Aca lo encontro TID: %d Prioridad: %d \n", hilo->tid, hilo->prioridad);
                    return hilo; 
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

bool existe_mutex(mutex_k* mutex_solic,t_list* lista_mutex_proceso)
{
    bool existe = false;
    mutex_k* aux;
    for(int i = 0; i < list_size(lista_mutex_proceso); i++)
    {
        aux = list_get(lista_mutex_proceso, i);
        if(strcmp(aux->nombre, mutex_solic->nombre) == 0) 
        {
            existe = true;
        }
    }
    return existe;
}
bool mutex_tomado_por_hilo(mutex_k* mutex, tcb* hilo)
{
    bool tomado = false;
    mutex_k* aux;
    for(int i=0; i < list_size(hilo->lista_mutex); i++)
    {
        aux = list_get(hilo->lista_mutex,i);
        if(strcmp(mutex->nombre, aux->nombre) == 0)
        {
            tomado = true;
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
nivel_prioridad* encontrar_por_nivel(t_list* lista_multinivel, int prioridad)
{
    //nivel_prioridad* aux;
    bool _existe_nivel(void* ptr)
    {
        nivel_prioridad* aux = (nivel_prioridad*) ptr;
        return aux->prioridad == prioridad;
    }

    nivel_prioridad* resultado = list_find(lista_multinivel, _existe_nivel);

    if(resultado != NULL)
    {
        return resultado;
    }
    return NULL;
}


nivel_prioridad* encontrar_nivel_mas_prioritario(t_list* multinivel)
{
    nivel_prioridad* mayor_prioridad;

    void* _max_prioridad(void* a, void* b) {
        nivel_prioridad* nivel_a = (nivel_prioridad*) a;
        nivel_prioridad* nivel_b = (nivel_prioridad*) b;
        
    return nivel_a->prioridad  <= nivel_b->prioridad ? nivel_a : nivel_b;
    }

    mayor_prioridad = list_get_maximum(multinivel, _max_prioridad);
    return mayor_prioridad;
}


// --------------------- Dump ---------------------

int bloquear_por_dump(tcb* hilo, int socket) 
{
    int finalizo_operacion = 0;
    t_paquete* dump = crear_paquete(DUMP_MEMORY);
	agregar_a_paquete_solo(dump, &(hilo->pcb_padre_tcb->pid), sizeof(int));
	agregar_a_paquete_solo(dump, &hilo->tid, sizeof(int));
	enviar_paquete(dump, socket);
	eliminar_paquete(dump);
     
    recv(socket,&finalizo_operacion, sizeof(int), MSG_WAITALL);
    
    return finalizo_operacion;
}
// --------------------- Mutex ---------------------
mutex_k* crear_mutex(char* nombre)
{
    mutex_k* mtx = (mutex_k*) malloc(sizeof(mutex_k)); 
    mtx->nombre = nombre;
    mtx->disponibilidad = true;

   return mtx;
}
void asignar_mutex_hilo(mutex_k* mutex,tcb* hilo)
{
    mutex->disponibilidad = false;
    mutex->hilo_poseedor = hilo;
    list_add(hilo->lista_mutex, mutex);
}


void liberar_mutexs_asociados(tcb* hilo)
{
    mutex_k* aux;
    int elementos = list_size(hilo->lista_mutex);
    for(int i = 0; i < elementos; i++)
    {
        aux = list_remove(hilo->lista_mutex, i);
        asignar_mutex_al_primer_bloqueado(aux);
    }
    list_destroy(hilo->lista_mutex);
}

void asignar_mutex_al_primer_bloqueado(mutex_k* mutex_solic)
{
    tcb* bloq_por_mutex;
    if(list_size(mutex_solic->bloqueados_por_mutex) > 0) 
	{
		bloq_por_mutex = list_remove(mutex_solic->bloqueados_por_mutex,0);
		asignar_mutex_hilo(mutex_solic, bloq_por_mutex);
	}
	else
	{
		mutex_solic->disponibilidad = true;
		mutex_solic->hilo_poseedor = NULL;
	}
}

// --------------------- funciones auxiliares ---------------------
void liberar_param_instruccion(instruccion* instrucc)
{
    for(int i = 0; i<list_size(instrucc->parametros); i++){
			free(list_get(instrucc->parametros,i));
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


void liberar_bloqueados_x_thread_join(tcb* hilo) 
{
    tcb* aux;
    if (hilo->block_join != NULL && list_size(hilo->block_join) > 0){
        for (int i = 0; i < list_size(hilo->block_join); i++){
            aux = list_remove(hilo->block_join, i);
            agregar_a_ready_segun_alg(aux);
        }
    }
    list_destroy(hilo->block_join);
}

void avisar_memoria_liberar_tcb(tcb* hilo)
{
    int socket = conectarMemoria();
    t_paquete* t_exit = crear_paquete(THREAD_EXIT);
    agregar_a_paquete_solo(t_exit, &(hilo->pcb_padre_tcb->pid), sizeof(int));
	agregar_a_paquete_solo(t_exit, &hilo->tid, sizeof(int));
	enviar_paquete(t_exit, socket);
    eliminar_paquete(t_exit);
    int confirmacion;
    recv(socket, &confirmacion, sizeof(int), MSG_WAITALL);
    close(socket);
}

void avisar_memoria_liberar_pcb(pcb* proc)
{
    int socket = conectarMemoria();
    t_paquete* p_exit = crear_paquete(PROCESS_EXIT);
	agregar_a_paquete(p_exit, &proc->pid, sizeof(int));
	enviar_paquete(p_exit, socket);
    eliminar_paquete(p_exit);

}
