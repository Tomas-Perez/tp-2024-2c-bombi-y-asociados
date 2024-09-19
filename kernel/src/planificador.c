#include "planificador.h"
int id_counter;

tcb* hilo_en_ejecucion; 
pthread_mutex_t m_hilo_en_ejecucion;
pthread_mutex_t m_lista_de_ready;
pthread_mutex_t m_regreso_de_cpu;
pthread_mutex_t m_hilo_a_ejecutar;
pthread_mutex_t m_lista_procesos_new;
t_list* lista_de_ready;
t_list* lista_procesos_new;
sem_t finalizo_un_proc;


void agregar_a_ready(tcb* hilo)
{
	pthread_mutex_lock(&m_lista_de_ready);
	list_add(lista_de_ready, hilo);
	pthread_mutex_unlock(&m_lista_de_ready);
	// sem_post(&hilos_en_ready)
}

void agregar_a_ready_prioridades(tcb* hilo)
{
	// TO DO
	// reordenar_lista_ready();
	//sem_post(&hilos_en_ready)
}
void pasar_a_running_tcb(tcb* tcb_listo)
{
   // mandar_tcb_dispatch(tcb_listo);
    pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo; 
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
    log_info(logger_kernel, "PID <%d> TID: <%d> - Estado Anterior: READY - Estado Actual: EXEC",
    hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
}

void mandar_tcb_dispatch(tcb* tcb_listo)
{
	// TO DO 
}
   


void planificador_corto_plazo_tcb()
{
	// semaforos!!!! binario para controlar la ejecucion y un contador para los hilos en ready
	//TO DO -> agus
	tcb* hilo_a_ejecutar;

	pthread_mutex_lock(&m_lista_de_ready);
    if (list_is_empty(lista_de_ready)) 
    {
    	log_error(logger_kernel, "Cola de ready esta vacia en planificador_corto_plazo\n");
    	pthread_mutex_unlock(&m_lista_de_ready);
		exit (1);
	}
        
    pthread_mutex_lock(&m_hilo_a_ejecutar);
	pthread_mutex_lock(&m_lista_de_ready);
	hilo_a_ejecutar = list_remove(lista_de_ready,0);
	pthread_mutex_unlock(&m_lista_de_ready);        
	
	if(hilo_a_ejecutar == NULL)
    {
        log_error(logger_kernel,"Hilo a ejecutar es NULL en planificador_corto_plazo\n");
        exit (1);
    }	
	pthread_mutex_unlock(&m_hilo_a_ejecutar);
		
    //solicitud_de_cpu();				
    
	if(strcmp(algoritmo_de_planificacion,"FIFO"))
	{
		
	}
	if(strcmp(algoritmo_de_planificacion,"PRIORIDADES"))
	{
		
	}
	if(strcmp(algoritmo_de_planificacion,"MULTINIVEL"))
	{
		// muerte TO DO (queda muy sombrio?)
	}
	//pasar_a_running_tcb(tcb_listo);
	
}



void atender_syscall()
{
	int motivo;
	int socket;
	int prioridad;

	instruccion* instrucc = malloc(sizeof(instruccion));
	instrucc->parametros = list_create();
	recibir_syscall_de_cpu(hilo_en_ejecucion, &motivo, instrucc);
	
	//BORRAR: es la misma que esta -> recibir_contexto_ejecc_de_cpu(proceso_en_ejecucion, &motivo ,instrucc);
	// pero sin los registros.  
	
	switch(motivo)
	{
		case PROCESS_CREATE:
		socket = conectarMemoria();

		char* archivo = list_get(instrucc->parametros, 0); //INICIAR_PROCESO1
		int tamanio = list_get(instrucc->parametros, 1);//255
		int prioridad = list_get(instrucc->parametros, 2);//1
		 char* path = generar_path_archivo(archivo);//home/utnso/INICIAR_PROCESO1.txt
		 
		pcb* proceso_nuevo = crear_pcb(prioridad, path, tamanio);
		//TO DO faltara un mutex
		list_add(lista_procesos_new, proceso_nuevo);
		pedir_memoria(socket);
		
		tcb* hilo_main = list_get(proceso_nuevo->lista_tcb, 0);       
		iniciar_hilo(hilo_main, socket, proceso_nuevo->path_proc);
		close(socket);
		break;
		case PROCESS_EXIT:
		/* esta syscall finalizará el PCB correspondiente al TCB que ejecutó la 
		instrucción, enviando todos sus TCBs asociados a la cola de EXIT. Esta 
		instrucción sólo será llamada por el TID 0 del proceso y le deberá indicar 
		a la memoria la finalización de dicho proceso. */
		
		if(hilo_en_ejecucion->tid == 0)
		{
			finalizar_hilos_proceso(hilo_en_ejecucion->pcb_padre_tcb);
			//finalizar_proceso(hilo_en_ejecucion->pid_padre_tcb,SUCCESS); VER cual de las 2 usamos
			// void finalizar_pcb_buscado(int pid, int motivo) tenemos esta funcion del tp anterior
			// aunque los procesos ahora solo pueden encontrarse en las listas de new, ready o exit
		}
		sem_post(&finalizo_un_proc);
		break;

		case THREAD_CREATE:
			prioridad = list_get(instrucc->parametros,1);
			pcb* proceso = hilo_en_ejecucion->pcb_padre_tcb;
			crear_tcb(proceso, prioridad);
			socket = conectarMemoria();
			/* THREAD_CREATE, esta syscall recibirá como parámetro de la CPU el nombre del 
			archivo de pseudocódigo que deberá ejecutar el hilo a crear y su prioridad. 
			Al momento de crear el nuevo hilo, deberá generar el nuevo TCB con un TID 
			autoincremental y poner al mismo en el estado READY. */

			
			iniciar_hilo(hilo_main, socket, proceso_nuevo->path_proc);
			close(socket);
			
		break;
		case THREAD_JOIN:
		break;
		case THREAD_CANCEL:
		break;
		case THREAD_EXIT:
			 finalizar_tcb(hilo_en_ejecucion);
		break;
		case MUTEX_CREATE:
		break;
		case MUTEX_LOCK:
		break;
		case MUTEX_UNLOCK:
		break;
		case DUMP_MEMORY:
		break;
		case IO:
		//usleep();
		break;
	}
}

