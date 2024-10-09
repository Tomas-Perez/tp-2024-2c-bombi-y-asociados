// quizas no esta en la mejor carpeta
/*#include <stdio.h>
#include<stdlib.h>*/

#include "utilsKernel.h"
t_log* logger_kernel;

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
	int id_counter = 0;
    

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
	 //cola de procesos
	lista_de_ready = list_create();
    lista_procesos_new = list_create();
    lista_multinivel = list_create();
    lista_finalizados = list_create();
     
}

void inicializar_cola_nivel_prioridad(nivel_prioridad* nuevo_nivel)
{
    nuevo_nivel->hilos_asociados = list_create();
    pthread_mutex_init(&nuevo_nivel->m_lista_prioridad, NULL);
}

// --------------------------- Pedidos memoria ---------------------------
void pedir_memoria(int socket)
{ 
   
    //TO DO -> mili :) 
    // con el socket sale la conexion
    // mandar a memoria el motivo PROCESS_CREATE,
    // mandar en este orden

    // 1) el pid 2) el tam proc 3) tam path 4) el path, los chicos se van a encargar de verificar si hay suficiente
    // memoria para abrir el archivo y si no volvemos a 
    // hacer recv para confirmacion, seguro nos mandan un bool
    // el segundo parámetro es el tamaño del proceso en Memoria y 
    //el tercer parámetro es la prioridad del hilo main (TID 0)

       pcb* proceso_nuevo = list_get(lista_procesos_new, 0);

        int pid = proceso_nuevo->pid;
        int tamanio = proceso_nuevo->tam_proc;
        char* path = proceso_nuevo->path_proc;
        int motivo = PROCESS_CREATE; //preguntar si solo es para PROCESS CREATE entonces mandarle un nombre mas descriptivo


        t_paquete* pedido_memoria = crear_paquete(motivo);
		agregar_a_paquete(pedido_memoria, &pid,sizeof(int));
        agregar_a_paquete(pedido_memoria, &tamanio, sizeof(int));
        //agregar_a_paquete // ponemos el tam del path?
        agregar_a_paquete(pedido_memoria, path, strlen(path) + 1);// PATH PREGUNTAR

        enviar_paquete(pedido_memoria,socket);
		eliminar_paquete(pedido_memoria);

        int confirmacion_mem_disponible = 0;
        recv(socket,&confirmacion_mem_disponible, sizeof(int), MSG_WAITALL);

        if(!confirmacion_mem_disponible)
        {

            sem_wait(&finalizo_un_proc);
            pedir_memoria(socket);	
        }
        else {
                list_remove(lista_procesos_new, 0);
        }
        
}

//void avisar_memoria(tcb hilo, char* path){
    // TO DO (creo, no se si la hicimos con otro nombre)
//}
//  --------------------------- Crear  --------------------------- 

pcb *crear_pcb(int prioridad_h_main, char* path, int tamanio)
{
    pcb* nuevo_pcb = (pcb *)malloc(sizeof(pcb));
    tcb* hilo_main;

    nuevo_pcb->tam_proc = tamanio;
    nuevo_pcb->path_proc = path;

    nuevo_pcb->contador_tid = 0;
    id_counter++;


    nuevo_pcb->lista_tcb = list_create();
    nuevo_pcb->lista_mutex_proc = list_create();
    if (nuevo_pcb == NULL)
    {
        free(nuevo_pcb);
        return NULL;
    }
    hilo_main = crear_tcb(nuevo_pcb, prioridad_h_main);
     

    return nuevo_pcb;
}

tcb* crear_tcb(pcb* proc_padre, int prioridad)
{
    tcb* nuevo_tcb = (tcb *)malloc(sizeof(tcb));  
    nuevo_tcb->tid = proc_padre->contador_tid;
    nuevo_tcb->pcb_padre_tcb = proc_padre;
    nuevo_tcb->prioridad = prioridad;
    nuevo_tcb->block_join = list_create();

    inicializar_registros(nuevo_tcb);
    proc_padre->contador_tid++;
    
    
    if (nuevo_tcb == NULL)
    {
        
        free(nuevo_tcb);
        return NULL;
    }
    list_add(proc_padre->lista_tcb, nuevo_tcb);
   
     return nuevo_tcb;
}

void crear_cola_nivel(int prioridad,tcb* hilo) // lo llamo asi x el nombre del alg pero creo una lista
{
    // crear la lista y agregar el hilo 
    // list add sorted para la cola multinivel
}


// -------------------------- Funciones planificador  --------------------------- 
void inicializar_hilos_planificacion()
{ // TO DO 
    pthread_t hilo_plani_corto,hilo_exitt;

	pthread_create(&hilo_plani_corto, NULL,(void*) planificador_corto_plazo,NULL);
    pthread_create(&hilo_exitt,NULL, (void*) hilo_exit, NULL);

	/*pthread_create(&hilo_plani_largo,NULL,(void*) planificador_largo_plazo,NULL);
	
	pthread_detach(hilo_plani_corto);
	pthread_detach(hilo_plani_largo);*/
	pthread_detach(hilo_exitt); 
    pthread_detach(hilo_plani_corto);
}

void mandar_tcb_dispatch(tcb* tcb_listo)
{
	// TO DO 
	t_paquete* tcb_a_dispatch = crear_paquete(OP_ENVIO_TCB);
	// Preguntar tomi funciones nuevas
	enviar_paquete(tcb_a_dispatch,conexion_dispatch);
}


void desalojar_hilo(int motivo)
{
    // VER solo copie la funcion de nuestro tp

    t_paquete *paquete_a_desalojar = crear_paquete(DESALOJAR_PROCESO);
	agregar_a_paquete_solo(paquete_a_desalojar,&motivo, sizeof(int));
	agregar_a_paquete_solo(paquete_a_desalojar,&hilo_en_ejecucion->tid, sizeof(int));
	enviar_paquete(paquete_a_desalojar, conexion_interrupt);
	eliminar_paquete(paquete_a_desalojar);
}

void* desalojar_por_RR(tcb* hilo)
{
    usleep(quantum*1000);
    if(hilo_en_ejecucion->tid == hilo->tid)
	{		
		desalojar_hilo(RR);
		log_info(logger_kernel,"TID: <%d> - Desalojado por fin de Quantum",hilo->tid);
		return;
	}
}

void agregar_a_ready_segun_alg(tcb* hilo)
{
	if(strcmp(algoritmo_de_planificacion, "MULTINIVEL") == 0)
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
			printf("TID: %i, motivo: %i", hilo->tid, *motivo);
            
		}
		else {printf("Codigo de Operacion de CPU incorrecto\n");
               
        }
}

void desempaquetar_parametros_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc){
		int tam;
		void* buffer = recibir_buffer(&tam, conexion_dispatch);
		int desplazamiento = 0;
		/*memcpy(&(proc->program_counter), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento+= sizeof(int)*/;
		memcpy(motivo, buffer + desplazamiento, sizeof(int));
		desplazamiento+= sizeof(int);
		printf("TID 2: %i, motivo 2: %i", hilo->tid, *motivo);
		memcpy(&(instrucc->cant_parametros), buffer + desplazamiento, sizeof(int));
		desplazamiento+= sizeof(int);
		for(int i = 0; i < instrucc->cant_parametros; i++){

			char* parametro; 
			int tamanio_parametro;
			memcpy(&(tamanio_parametro), buffer + desplazamiento, sizeof(int));
			desplazamiento+= sizeof(int);
			parametro= (char*) calloc(1,sizeof(tamanio_parametro));
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
			desplazamiento+= tamanio_parametro;

			list_add(instrucc->parametros, parametro);
            
		}
        
		free(buffer);
	}

// --------------------- Iniciar hilos ---------------------
void iniciar_hilo(tcb* hilo, int conexion_memoria, char* path){
        
        uint32_t size_path_hilo = sizeof(path);
        // TO DO opcional: usar funcion tomi
        t_paquete *paquete = crear_paquete(INICIAR_HILO);
        agregar_a_paquete_solo(paquete, &(hilo->pcb_padre_tcb->pid), sizeof(uint32_t));
        agregar_a_paquete_solo(paquete, &(hilo->tid), sizeof(uint32_t));
        agregar_a_paquete_solo(paquete, &(size_path_hilo), sizeof(uint32_t));
        agregar_a_paquete(paquete, path, strlen(path) + 1);
        enviar_paquete(paquete, conexion_memoria);
        eliminar_paquete(paquete);

        bool confirmacion;
        recv(conexion_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);

        if (confirmacion)
        {
           // agregue este sem post xq si no nunca podia pasar el semwait de agregar a nuevos!!
            if (hilo == NULL)
            {
                printf("HILO trucho1");
            }
            if(strcmp(algoritmo_de_planificacion, "MULTINIVEL")== 0)
            {
                agregar_a_ready_multinivel(hilo);
            }
            else
            {
                agregar_a_ready(hilo);
            }
            
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
    list_destroy(proc->lista_tcb);
    free(proc->path_proc);

}


void finalizar_hilos_proceso(pcb* proceso)  // PARA PROCESS_EXIT
{
   
    tcb* hilo_a_finalizar;
    while(list_is_empty(proceso->lista_tcb) == 0)
    {
        hilo_a_finalizar = list_remove(proceso->lista_tcb,0);
        finalizar_tcb(hilo_a_finalizar);
    }
}

void finalizar_tcb(tcb* hilo_a_finalizar)
{
    pthread_mutex_lock(&m_lista_finalizados);
    list_add(lista_finalizados,hilo_a_finalizar);
    pthread_mutex_unlock(&m_lista_finalizados);

    sacar_de_lista_pcb(hilo_a_finalizar);
    sem_post(&hilos_en_exit);
    log_info(logger_kernel,"Finaliza el hilo <%d>", hilo_a_finalizar->tid);
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
		printf("BORRAR: en hilo_exit-> termino todo");
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
        if(strcmp(aux->nombre, mutex_solic->nombre) == 0) //Comparo x nombre, supongo q no nos van a dar 2 mutex q se llamen igual en el mismo proc
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
        
    return nivel_a->prioridad  >= nivel_b->prioridad ? nivel_a : nivel_b;
    }

    mayor_prioridad = list_get_maximum(multinivel, _max_prioridad);
    return mayor_prioridad;
}


// --------------------- Dump ---------------------

void bloquear_por_dump(tcb* hilo) 
{
    // TO DO 
    // aca contemplamos este caso tmbn:
    // memoria confirme la finalización de la operación, en caso de error, el proceso se enviará a EXIT.
    // o lo hacemos dentro del case de la syscall?
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
    // TO DO
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

void sacar_de_lista_pcb(tcb* hilo_a_sacar)
{
    pcb* proc_padre = hilo_a_sacar->pcb_padre_tcb;
    bool lo_encontro;
    lo_encontro = list_remove_element(proc_padre->lista_tcb, hilo_a_sacar);

    if(lo_encontro != 0)
    {
        proc_padre->contador_tid--;
        if(proc_padre <= 0)
        {
            finalizar_proceso(proc_padre);
        }
    }

}


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
    // TO DO pid y tid crear buffer y despues usar
    // buffer_add_uint32(t_buffer *buffer, uint32_t data) THREAD_EXIT 
}

void avisar_memoria_liberar_pcb(pcb* proc)
{
    // TO DO --> ver si conviene que devuelva un bool q avise q se libero bien PROCESS_EXIT

    //pid 
}