#include "planificador.h"
int id_counter;

tcb* hilo_en_ejecucion; 
pcb* proceso_en_ejecucion; //VER SI ESTA BIEN ACA 
pthread_mutex_t m_hilo_en_ejecucion;
pthread_mutex_t m_proceso_en_ejecucion;
pthread_mutex_t m_proceso_a_ejecutar;
pthread_mutex_t m_lista_de_ready;
pthread_mutex_t m_regreso_de_cpu;
t_list* lista_de_ready;


void pasar_a_running_tcb(tcb* tcb_listo)
{
   // mandar_tcb_dispatch(tcb_listo);
    pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo; 
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
    log_info(logger_kernel, "PID <%d> TID: <%d> - Estado Anterior: READY - Estado Actual: EXEC",
    proceso_en_ejecucion->pid, tcb_listo->tid);
}

 void pasar_a_running_pcb(pcb* proceso_listo){
		
		pthread_mutex_lock(&m_proceso_en_ejecucion);
		proceso_en_ejecucion = proceso_listo; 
		pthread_mutex_unlock(&m_proceso_en_ejecucion);
        log_info(logger_kernel, "PID: <%d> - Estado Anterior: READY - Estado Actual: EXEC", proceso_listo->pid);

        // aca llamar a planificador corto plazo que llame a pasar_a_running_tcb
}

void mandar_tcb_dispatch(tcb* tcb_listo){
	// TODO 
}

void planificador_corto_plazo_pcb()
{
    while(1)
    {
		pcb* proceso_a_ejecutar;
        //sem_wait(&binario_corto_plazo); VER COMO LLAMAMOS SEMAFOROS
	  	//sem_wait(&cont_procesos_en_ready);
		pthread_mutex_lock(&m_lista_de_ready);
        if (list_is_empty(lista_de_ready)) 
        {

      		log_error(logger_kernel, "Cola de ready está vacía en planificador_corto_plazo\n");
        	pthread_mutex_unlock(&m_lista_de_ready);
			return 1;
		}
        
        pthread_mutex_lock(&m_proceso_a_ejecutar);
		pthread_mutex_lock(&m_lista_de_ready);
		proceso_a_ejecutar = list_remove(lista_de_ready,0);
		pthread_mutex_unlock(&m_lista_de_ready);        
				
        if(proceso_a_ejecutar == NULL)
        {
            log_error(logger_kernel,"Proceso a ejecutar es NULL en planificador_corto_plazo\n");
       	    return 1;
    	}	
        
		pasar_a_running_pcb(proceso_a_ejecutar);
		pthread_mutex_unlock(&m_proceso_a_ejecutar);
		
        //solicitud_de_cpu();				
    }
}

void planificador_corto_plazo_tcb()
{
	// semaforos!!!!
	if(strcmp(algoritmo_de_planificacion,"FIFO"))
	{

	}
	if(strcmp(algoritmo_de_planificacion,"PRIORIDADES"))
	{

	}
	if(strcmp(algoritmo_de_planificacion,"MULTINIVEL"))
	{

	}
	//pasar_a_running_tcb(tcb_listo);
}



void atender_syscall()
{
	int motivo;
	//recibir_contexto_ejecc_de_cpu(proceso_en_ejecucion, &motivo ,instrucc);
	switch(motivo)
	{
		case PROCESS_CREATE:
		/*void* socket;
		pthread_create(&t1, NULL, (void *)conectarMemoria, NULL);
		pthread_join(&t1, ); suponemos que se va a usar de uno la conexion a memoria*/
		
		int socket = conectarMemoria();
		// hacemos el pedido
		close(socket);
		break;
		case PROCESS_EXIT:
		break;
		case THREAD_CREATE:
		break;
		case THREAD_JOIN:
		break;
		case THREAD_CANCEL:
		break;
		case THREAD_EXIT:
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

