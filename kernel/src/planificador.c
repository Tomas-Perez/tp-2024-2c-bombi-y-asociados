#include "planificador.h"
int id_counter;
int indice;

tcb* hilo_en_ejecucion; 
pthread_mutex_t m_hilo_en_ejecucion;
pthread_mutex_t m_lista_de_ready;
pthread_mutex_t m_regreso_de_cpu;
pthread_mutex_t m_hilo_a_ejecutar;
pthread_mutex_t m_lista_procesos_new;
pthread_mutex_t m_indice;
pthread_mutex_t m_lista_multinivel; 
pthread_mutex_t m_lista_finalizados;

t_list* lista_de_ready;
t_list* lista_procesos_new;
t_list* lista_multinivel;
t_list* lista_finalizados;
sem_t finalizo_un_proc;
sem_t hilos_en_exit;


void agregar_a_ready(tcb* hilo)
{
	pthread_mutex_lock(&m_lista_de_ready);
	list_add(lista_de_ready, hilo);
	pthread_mutex_unlock(&m_lista_de_ready);
	// sem_post(&hilos_en_ready)
	
}

void agregar_a_ready_multinivel(tcb* hilo)
{
	nivel_prioridad* cola_nivel;
	// chequear si existe alguna cola con esa prioridad
	cola_nivel = encontrar_por_nivel(lista_multinivel, hilo->prioridad);
	if(cola_nivel == NULL)
	{
		crear_cola_nivel(hilo->prioridad, hilo);
		inicializar_cola_nivel_prioridad(cola_nivel);
	}
	else
	{
		//list_add a la cola de esa prioridad		
		pthread_mutex_lock(&(cola_nivel->m_lista_prioridad));
		list_add(cola_nivel->hilos_asociados, hilo);
		pthread_mutex_unlock(&(cola_nivel->m_lista_prioridad));
	}
}

tcb* hilo_prioritario_en_ready(){
  

	pthread_mutex_lock(&m_lista_de_ready);
	tcb*  hilo_elegido = list_get(lista_de_ready, 0);
	pthread_mutex_unlock(&m_lista_de_ready);

    tcb* tcb_aux;
	pthread_mutex_lock(&m_indice);
    indice = 0;
    pthread_mutex_unlock(&m_indice);    
    for(int i=1; i < list_size(lista_de_ready); i++){
        pthread_mutex_lock(&m_lista_de_ready);
       tcb_aux = list_get(lista_de_ready, i);
        pthread_mutex_unlock(&m_lista_de_ready);

        if(hilo_elegido->prioridad > tcb_aux->prioridad){
            hilo_elegido = tcb_aux;
            pthread_mutex_lock(&m_indice);
            indice = i; 
            pthread_mutex_unlock(&m_indice);    
        }
    }

    return hilo_elegido;
}

tcb* elegir_segun_prioridades(){
    tcb* hilo_elegido = hilo_prioritario_en_ready();

	pthread_mutex_lock(&m_lista_de_ready);
    list_remove(lista_de_ready, indice);
    pthread_mutex_unlock(&m_lista_de_ready);

    return hilo_elegido;
}


void pasar_a_running_tcb(tcb* tcb_listo)
{
   	mandar_tcb_dispatch(tcb_listo);
    pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo; 
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
    log_info(logger_kernel, "PID <%d> TID: <%d> - Estado Anterior: READY - Estado Actual: EXEC",
    hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
}

//esta me la invente puede q no sea asi 
void pasar_a_running_tcb_prioridades(){

	tcb* tcb_listo = elegir_segun_prioridades();
	pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo; 
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
	mandar_tcb_dispatch(tcb_listo);
    log_info(logger_kernel, "PID <%d> TID: <%d> - Estado Anterior: READY - Estado Actual: EXEC",
    hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
}

void mandar_tcb_dispatch(tcb* tcb_listo)
{
	// TO DO 
	t_paquete* tcb_a_dispatch = crear_paquete(OP_ENVIO_TCB);
	// Preguntar tomi funciones nuevas
	enviar_paquete(tcb_a_dispatch,conexion_dispatch);
}
   


void planificador_corto_plazo()
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
        
    
		
    //solicitud_de_cpu();				
    
	if(strcmp(algoritmo_de_planificacion,"FIFO"))
	{
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
		pasar_a_running_tcb(hilo_a_ejecutar);
	}
	if(strcmp(algoritmo_de_planificacion,"PRIORIDADES"))
	{
		pasar_a_running_tcb_prioridades();
	}
	if(strcmp(algoritmo_de_planificacion,"MULTINIVEL"))
	{
		// muerte TO DO (queda muy sombrio?)
	}
	
	
}
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


void atender_syscall()
{
	int motivo;
	int socket;
	int prioridad;
	char* archivo;
	instruccion* instrucc = malloc(sizeof(instruccion));
	instrucc->parametros = list_create();
	recibir_syscall_de_cpu(hilo_en_ejecucion, &motivo, instrucc);
	t_list* lista_mutex_proceso;
	mutex_k* mutex_solic;
	//BORRAR: es la misma que esta -> recibir_contexto_ejecc_de_cpu(proceso_en_ejecucion, &motivo ,instrucc);
	// pero sin los registros.  
	
	switch(motivo)
	{
		case PROCESS_CREATE:
		socket = conectarMemoria();

		archivo = list_get(instrucc->parametros, 0); //INICIAR_PROCESO1
		int tamanio =  list_get(instrucc->parametros, 1);//255
		prioridad =  list_get(instrucc->parametros, 2);//1
		printf("PRUEBA: tamanio: %d prioridad %d\n", tamanio, prioridad);
		char* path = generar_path_archivo(archivo);//home/utnso/INICIAR_PROCESO1.txt
		 
		pcb* proceso_nuevo = crear_pcb(prioridad, path, tamanio);
		pthread_mutex_lock(&m_lista_procesos_new);
		list_add(lista_procesos_new, proceso_nuevo);
		pthread_mutex_unlock(&m_lista_procesos_new);
		pedir_memoria(socket);
		
		tcb* hilo_main = list_get(proceso_nuevo->lista_tcb, 0);       
		iniciar_hilo(hilo_main, socket, proceso_nuevo->path_proc);
		close(socket);
		break;
		case PROCESS_EXIT:
			
		if(hilo_en_ejecucion->tid == 0)
		{
			finalizar_proceso(hilo_en_ejecucion->pcb_padre_tcb);
		}
		// sem_post(&finalizo_un_proc); VER: creo q este semaforo no se usa en ninguna parte?
		break;

		case THREAD_CREATE:
			archivo = list_get(instrucc->parametros, 0);
			prioridad = list_get(instrucc->parametros,1);
			pcb* proceso = hilo_en_ejecucion->pcb_padre_tcb;
			tcb* hilo = crear_tcb(proceso, prioridad);
			socket = conectarMemoria();
			/* THREAD_CREATE, esta syscall recibirá como parámetro de la CPU el nombre del 
			archivo de pseudocódigo que deberá ejecutar el hilo a crear y su prioridad. 
			Al momento de crear el nuevo hilo, deberá generar el nuevo TCB con un TID 
			autoincremental y poner al mismo en el estado READY. */

			
			iniciar_hilo(hilo, socket, archivo);
			// TO DO: poner en ready
			close(socket);
			
		break;
		case THREAD_JOIN:
		// 	TO DO
		break;
		case THREAD_CANCEL:
			/*esta syscall recibe como parámetro un TID con el objetivo de finalizarlo pasando al mismo al estado EXIT.
			 Se deberá indicar a la Memoria la finalización de dicho hilo. 
			 En caso de que el TID pasado por parámetro no exista o ya haya finalizado, 
			 esta syscall no hace nada. Finalmente, el hilo que la invocó continuará su ejecución.*/
			 int tid = list_get(instrucc->parametros, 0);
			 tcb* hilo_a_finalizar;
			hilo_a_finalizar = buscar_TID(hilo_en_ejecucion,tid);
			if(hilo_a_finalizar != NULL){
				//asegurarse que no esta en exit para no finalizarlo 2 veces 
				// por ahi mejor fijarse en finalizar_tcb
				finalizar_tcb(hilo_a_finalizar);
			}

		break;
		case THREAD_EXIT:
			finalizar_tcb(hilo_en_ejecucion);
		break;
		case MUTEX_CREATE:
			mutex_k* nuevo_mutex;
			char* nombre_mutex = list_get(instrucc->parametros,0); //Borrar: medio al pedo pero para q se entienda mas
			nuevo_mutex= crear_mutex(nombre_mutex);
			list_add(hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc, nuevo_mutex);

		break;
		case MUTEX_LOCK:
				lista_mutex_proceso = hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc;
				mutex_solic = list_get(instrucc->parametros, 0);
				if(existe_mutex(mutex_solic, lista_mutex_proceso) != false)
				{
					if(mutex_solic->disponibilidad = true)
					{
						asignar_mutex_hilo(mutex_solic, hilo_en_ejecucion);
					}
					else
					{
						// TO DO AGUS: sacalo de la cola de ready para que no sea planificable 
						list_add(mutex_solic->bloqueados_por_mutex, mutex_solic);
					}
				}
				else
				{
					// TO DO: Preguntar que pasa si no existe el mutex en el proc
					// finalizar
				}
		break;
		case MUTEX_UNLOCK:
			
			mutex_k* mutex_solic = list_get(instrucc->parametros, 0);
			lista_mutex_proceso = hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc;
			tcb* bloq_por_mutex;
			
			if(existe_mutex(mutex_solic, lista_mutex_proceso) != false)
			{
				if(mutex_tomado_por_hilo(mutex_solic, hilo_en_ejecucion) != false)
				{
					if(list_size(mutex_solic->bloqueados_por_mutex) > 0) // muchos ifs? :/
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

			}
			
		break;
		case DUMP_MEMORY:
			socket = conectarMemoria();
			t_paquete* dump = crear_paquete(DUMP_MEMORY);
			agregar_a_paquete(dump, &(hilo_en_ejecucion->pcb_padre_tcb->pid), sizeof(int));
			agregar_a_paquete(dump, &hilo_en_ejecucion->tid, sizeof(int));
			enviar_paquete(dump, socket);

			bloquear_por_dump(hilo_en_ejecucion);
			
			close(socket);
		break;
		case IO:
			int cant_seg_duerme = list_get(instrucc->parametros, 0);
			usleep(cant_seg_duerme);
		break;
	}
}

