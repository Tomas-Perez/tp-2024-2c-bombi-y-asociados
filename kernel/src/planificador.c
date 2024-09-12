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
	// TO DO 
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
			exit (1);
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
	// semaforos!!!! TO DO -> agus
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
	int socket;
	int prioridad;

	instruccion* instrucc = malloc(sizeof(instruccion));
	instrucc->parametros = list_create();
	recibir_syscall_de_cpu(proceso_en_ejecucion, &motivo, instrucc);
	//BORRAR: es la misma que esta -> recibir_contexto_ejecc_de_cpu(proceso_en_ejecucion, &motivo ,instrucc);
	// pero sin los registros.  
	
	switch(motivo)
	{
		case PROCESS_CREATE:
		socket = conectarMemoria();
		pcb* proceso_nuevo = crear_pcb(prioridad);
        pedir_memoria(proceso_nuevo, socket);        
	
		close(socket);
		break;
		case PROCESS_EXIT:
		/* esta syscall finalizará el PCB correspondiente al TCB que ejecutó la 
		instrucción, enviando todos sus TCBs asociados a la cola de EXIT. Esta 
		instrucción sólo será llamada por el TID 0 del proceso y le deberá indicar 
		a la memoria la finalización de dicho proceso. */
		
		break;

		case THREAD_CREATE:
			socket = conectarMemoria();
			tcb* hilo_nuevo;

			/* THREAD_CREATE, esta syscall recibirá como parámetro de la CPU el nombre del 
			archivo de pseudocódigo que deberá ejecutar el hilo a crear y su prioridad. 
			Al momento de crear el nuevo hilo, deberá generar el nuevo TCB con un TID 
			autoincremental y poner al mismo en el estado READY. */

			//hilo_nuevo = crear_tcb(proceso_en_ejec, int prioridad); REVISAR, supongo que el proceso padre va a ser el que esta en ejec
			// una vez que tengamos hecho el rec contexto se puede tener la prioridad
			// algun hilo va a llamar esta syscall, este hilo va a tener un padre y ahora
			// van a compartir padre el hilo que llama y el que nace
			close(socket);
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

