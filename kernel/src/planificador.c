#include "planificador.h"
int id_counter;
int indice;

tcb *hilo_en_ejecucion;
pthread_mutex_t m_hilo_en_ejecucion;
pthread_mutex_t m_lista_de_ready;
pthread_mutex_t m_lista_de_bloqueados;
pthread_mutex_t m_regreso_de_cpu;
pthread_mutex_t m_hilo_a_ejecutar;
pthread_mutex_t m_lista_procesos_new;
pthread_mutex_t m_indice;
pthread_mutex_t m_lista_multinivel;
pthread_mutex_t m_lista_finalizados;
//pthread_mutex_t m_lista_prioridad;
pthread_mutex_t m_syscall_solicitada;
pthread_mutex_t m_lista_io;
pthread_mutex_t m_bloqueados_por_dump;

t_list *bloqueados_por_dump;
t_list *lista_de_ready;
t_list *lista_procesos_new;
t_list *lista_multinivel;
t_list *lista_finalizados;
t_list *lista_bloqueados;
t_list *lista_io;
sem_t finalizo_un_proc;
sem_t hilos_en_exit;
sem_t hilos_en_ready;
sem_t binario_corto_plazo;

void agregar_a_ready(tcb *hilo)
{
	pthread_mutex_lock(&m_lista_de_ready);
	list_add(lista_de_ready, hilo);
	pthread_mutex_unlock(&m_lista_de_ready);
	sem_post(&hilos_en_ready);
}

void agregar_a_ready_multinivel(tcb *hilo)
{
	if (lista_multinivel == NULL)
	{
		printf("Error: lista_multinivel es NULL\n");
		return NULL;
	}
	else
	{
		// printf("Lista inicializada correctamente\n");
	}
	pthread_mutex_lock(&m_lista_multinivel);
	nivel_prioridad *cola_nivel = encontrar_por_nivel(lista_multinivel, hilo->prioridad);
	pthread_mutex_unlock(&m_lista_multinivel);
	if (cola_nivel == NULL)
	{
		int prioridad_hilo = (hilo->prioridad);
		crear_cola_nivel(prioridad_hilo, hilo);
	}
	else
	{
		pthread_mutex_lock(&(cola_nivel->m_lista_prioridad));
		list_add(cola_nivel->hilos_asociados, hilo);
		pthread_mutex_unlock(&(cola_nivel->m_lista_prioridad));
	}
	sem_post(&hilos_en_ready);
}

void planificador_corto_plazo()
{
	while (1)
	{

		sem_wait(&binario_corto_plazo);
		sem_wait(&hilos_en_ready);
		tcb *hilo_a_ejecutar;

		if (strcmp(algoritmo_de_planificacion, "FIFO") == 0)
		{
			verificar_lista_ready(lista_de_ready);
			pthread_mutex_lock(&m_hilo_a_ejecutar);
			pthread_mutex_lock(&m_lista_de_ready);
			hilo_a_ejecutar = list_remove(lista_de_ready, 0);
			pthread_mutex_unlock(&m_lista_de_ready);

			if (hilo_a_ejecutar == NULL)
			{
				log_error(logger_kernel, "Hilo a ejecutar es NULL en planificador_corto_plazo\n");
				exit(1);
			}
			pthread_mutex_unlock(&m_hilo_a_ejecutar);
			pasar_a_running_tcb_con_syscall(hilo_a_ejecutar);
			//atender_syscall();
		}
		if (strcmp(algoritmo_de_planificacion, "PRIORIDADES") == 0)
		{
			verificar_lista_ready(lista_de_ready);
			pasar_a_running_tcb_prioridades();
			//atender_syscall();
		}
		if (strcmp(algoritmo_de_planificacion, "CMN") == 0)
		{
			pthread_mutex_lock(&m_lista_multinivel);
			if(list_size(lista_multinivel) > 0){
			pthread_mutex_unlock(&m_lista_multinivel);

			nivel_prioridad *mayor_nivel;
			mayor_nivel = encontrar_nivel_mas_prioritario(lista_multinivel);
			if(mayor_nivel != NULL)
			{
			printf("Prioridad mayor nivel %d\n", mayor_nivel->prioridad);
			pthread_mutex_lock(&(mayor_nivel->m_lista_prioridad));
			if (list_is_empty(mayor_nivel->hilos_asociados) == false)
			{
				hilo_a_ejecutar = list_remove(mayor_nivel->hilos_asociados, 0);
				if (list_size(mayor_nivel->hilos_asociados) == 0)
				{
					bool loBorra = list_remove_element(lista_multinivel, mayor_nivel);
					printf("Bool lo borra %b \n", loBorra);
					free(mayor_nivel);
				}
			}
			pthread_mutex_unlock(&(mayor_nivel->m_lista_prioridad));

			pthread_t tround_robin;
			pasar_a_running_tcb(hilo_a_ejecutar);
			pthread_create(&tround_robin, NULL, (void *)desalojar_por_RR, (void *)hilo_a_ejecutar);
			printf("antes de la syscall \n");
			atender_syscall();
			printf("despues de la syscall \n");

			pthread_detach(tround_robin);
			}
			//pthread_cancel(tround_robin);
		}
		} else {
			pthread_mutex_unlock(&m_lista_multinivel);
			printf("No hay hilos en ready.\n");
		}
	}
}

void pasar_a_running_tcb(tcb *tcb_listo)
{
	mandar_tcb_dispatch(tcb_listo);
	pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo;
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
	log_info(logger_kernel, "PID <%d> TID: <%d> - Estado Anterior: READY - Estado Actual: EXEC",
			 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
	//atender_syscall();
}

void pasar_a_running_tcb_con_syscall(tcb *tcb_listo)
{
	mandar_tcb_dispatch(tcb_listo);
	pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo;
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
	log_info(logger_kernel, "PID <%d> TID: <%d> - Estado Actual: EXEC",
			 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
	atender_syscall();
}

void pasar_a_running_tcb_prioridades()
{

	tcb *tcb_listo = elegir_segun_prioridades();
	pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo;
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
	mandar_tcb_dispatch(tcb_listo);
	log_info(logger_kernel, "PID <%d> TID: <%d>  - Estado Anterior: READY - Estado Actual: EXEC",
			 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
	atender_syscall();
}

tcb *hilo_prioritario_en_ready()
{

	pthread_mutex_lock(&m_lista_de_ready);
	tcb *hilo_elegido = list_get(lista_de_ready, 0);
	pthread_mutex_unlock(&m_lista_de_ready);

	tcb *tcb_aux;
	pthread_mutex_lock(&m_indice);
	indice = 0;
	pthread_mutex_unlock(&m_indice);
	for (int i = 1; i < list_size(lista_de_ready); i++)
	{
		pthread_mutex_lock(&m_lista_de_ready);
		tcb_aux = list_get(lista_de_ready, i);
		pthread_mutex_unlock(&m_lista_de_ready);

		if (hilo_elegido->prioridad > tcb_aux->prioridad)
		{
			hilo_elegido = tcb_aux;
			pthread_mutex_lock(&m_indice);
			indice = i;
			pthread_mutex_unlock(&m_indice);
		}
	}

	return hilo_elegido;
}

tcb *elegir_segun_prioridades()
{
	tcb *hilo_elegido = hilo_prioritario_en_ready();

	pthread_mutex_lock(&m_lista_de_ready);
	pthread_mutex_lock(&m_indice);
	list_remove(lista_de_ready, indice);
	pthread_mutex_unlock(&m_indice);
	pthread_mutex_unlock(&m_lista_de_ready);

	return hilo_elegido;
}


void atender_syscall()
{
	int motivo;
	int socket;
	int prioridad;
	int tid;
	char *archivo;
	instruccion *instrucc = malloc(sizeof(instruccion));
	instrucc->parametros = list_create();
	recibir_syscall_de_cpu(hilo_en_ejecucion, &motivo, instrucc);
	t_list *lista_mutex_proceso;
	mutex_k *mutex_solic;

	log_info(logger_kernel, "## (PID <%d> TID: <%d> ) - Solicitó syscall: <%s>",
			 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid, op_code_to_string(motivo));
	/*pthread_mutex_lock(&m_syscall_solicitada);
	syscall_solicitada = 1;
	pthread_mutex_unlock(&m_syscall_solicitada);*/
	switch (motivo)
	{
	case RR: 
		//printf("entro en syscall");
		sem_post(&binario_corto_plazo);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		if(hilo_en_ejecucion != NULL)
		{
			pthread_mutex_unlock(&m_hilo_en_ejecucion);
       		 agregar_a_ready_multinivel(hilo_en_ejecucion);
		}
		pthread_mutex_unlock(&m_hilo_en_ejecucion);
	break;
	case PROCESS_CREATE:
		socket = conectarMemoria();

		archivo = list_get(instrucc->parametros, 0);
		char *tamanio = list_get(instrucc->parametros, 1);
		char *prioridad_PC = list_get(instrucc->parametros, 2);
		int tam = atoi(tamanio);
		int priori = atoi(prioridad_PC);

		printf("PRUEBA: %s tamanio: %d prioridad %d\n", archivo, tam, priori);


		pcb *proceso_nuevo = crear_pcb(priori, archivo, tam, socket);

		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);

		free(archivo);
		close(socket);
		break;

	case PROCESS_EXIT:
		 pthread_mutex_lock(&m_hilo_en_ejecucion);
		if (hilo_en_ejecucion->tid == 0)
		{
			
			pthread_mutex_unlock(&m_hilo_en_ejecucion);
			finalizar_proceso(hilo_en_ejecucion->pcb_padre_tcb);
			pthread_mutex_lock(&m_hilo_en_ejecucion);
			hilo_en_ejecucion=NULL;	
			pthread_mutex_unlock(&m_hilo_en_ejecucion);	
			
		}  pthread_mutex_unlock(&m_hilo_en_ejecucion);
		break;

	case THREAD_CREATE:
		archivo = list_get(instrucc->parametros, 0);
		prioridad = atoi(list_get(instrucc->parametros, 1));
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		pcb *proceso = hilo_en_ejecucion->pcb_padre_tcb;
		 pthread_mutex_unlock(&m_hilo_en_ejecucion);
		tcb *hilo = crear_tcb(proceso, prioridad);
		socket = conectarMemoria();

		printf("%s priori: %d\n", archivo, prioridad);
	
		iniciar_hilo(hilo, socket, archivo);

		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		close(socket);
		free(archivo);
		break;
	case THREAD_JOIN:
		char *tid_j = list_get(instrucc->parametros, 0);
		tid = atoi(tid_j);
		tcb *tcb_invocado = buscar_tid(hilo_en_ejecucion->pcb_padre_tcb->lista_tcb, tid);

		if (tcb_invocado != NULL)
		{
			
			pthread_mutex_lock(&m_hilo_a_ejecutar);

			//	if(strcmp(algoritmo_de_planificacion,"CMN") != 0) {
			buscar_hilos_listas(hilo_en_ejecucion, hilo_en_ejecucion->tid);
			//}

			log_info(logger_kernel, "## (PID <%d> : TID <%d>) - Bloqueado por: <PTHREAD_JOIN>",
					 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);

			list_add(tcb_invocado->block_join, hilo_en_ejecucion);

			// hilo_en_ejecucion = tcb_invocado;
			pthread_mutex_unlock(&m_hilo_a_ejecutar);
			pthread_mutex_lock(&m_hilo_en_ejecucion);
			hilo_en_ejecucion=NULL;
			pthread_mutex_unlock(&m_hilo_en_ejecucion);
			//agregar_a_ready_segun_alg(tcb_invocado);
			sem_post(&binario_corto_plazo);
		}
		else
		{
			log_info(logger_kernel, "No se encontró el hilo");
			pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		}

		break;

	case THREAD_CANCEL:
		char *tid_c = list_get(instrucc->parametros, 0);
		tid = atoi(tid_c);
		tcb *hilo_a_finalizar;
		hilo_a_finalizar = buscar_hilos_listas(hilo_en_ejecucion, tid);
		if (hilo_a_finalizar != NULL)
		{
			finalizar_tcb(hilo_a_finalizar);
		}
		else
		{
			log_info(logger_kernel, "No se encontró el hilo");
		}
		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		break;
	case THREAD_EXIT:
		finalizar_tcb(hilo_en_ejecucion);
		sem_post(&binario_corto_plazo);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		hilo_en_ejecucion=NULL;
		pthread_mutex_unlock(&m_hilo_en_ejecucion);
		break;
	case MUTEX_CREATE:
		mutex_k *nuevo_mutex;
		char *nombre_mutex = list_get(instrucc->parametros, 0);
		nuevo_mutex = crear_mutex(nombre_mutex);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		list_add(hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc, nuevo_mutex);
		 pthread_mutex_unlock(&m_hilo_en_ejecucion);
		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		break;
	case MUTEX_LOCK:
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		lista_mutex_proceso = hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc;
		 pthread_mutex_unlock(&m_hilo_en_ejecucion);
		char* nombre_mutex_solic;
		nombre_mutex_solic = list_get(instrucc->parametros, 0);
		printf("tamanio lista mutex del proc: %d ", list_size(lista_mutex_proceso));
		if(list_size(lista_mutex_proceso) > 0){
			 mutex_k *aux = existe_mutex_por_nombre(nombre_mutex_solic, lista_mutex_proceso);
			if ( aux != NULL)
			{
				if (aux->disponibilidad == true)
				{
					asignar_mutex_hilo(aux, hilo_en_ejecucion);
					pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
				}
				else
				{
					buscar_hilos_listas(hilo_en_ejecucion, hilo_en_ejecucion->tid);
					list_add(aux->bloqueados_por_mutex, hilo_en_ejecucion);
					log_info(logger_kernel, "## (PID <%d> : TID <%d>) - Bloqueado por: <MUTEX>",
							hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
					pthread_mutex_lock(&m_hilo_en_ejecucion);
					hilo_en_ejecucion = NULL;
					pthread_mutex_unlock(&m_hilo_en_ejecucion);
					sem_post(&binario_corto_plazo);
				}
			}
			else
			{
				finalizar_tcb(hilo_en_ejecucion);
				sem_post(&binario_corto_plazo);
			}
			}
			else
			{
				finalizar_tcb(hilo_en_ejecucion);
				sem_post(&binario_corto_plazo);
			}
		break;
	case MUTEX_UNLOCK:
		char* nombre_mutex_solici;
		nombre_mutex_solici = list_get(instrucc->parametros, 0);
		//mutex_k *mutex_solic = list_get(instrucc->parametros, 0);
		lista_mutex_proceso = hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc;
		 mutex_k *aux_m = existe_mutex_por_nombre(nombre_mutex_solici, lista_mutex_proceso);
		if (aux_m != NULL)
		{
			bool esTomado = mutex_por_nombre_tomado_por_hilo(nombre_mutex_solici, hilo_en_ejecucion);
			if (esTomado != false)
			{
				asignar_mutex_al_primer_bloqueado(aux_m);
			}
		}
		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		break;
	case DUMP_MEMORY:
		socket = conectarMemoria();

		tcb *hilo_dump = hilo_en_ejecucion;
		

		int rta = bloquear_por_dump(hilo_dump, socket);
		close(socket);
		pthread_mutex_lock(&m_bloqueados_por_dump);
		hilo_dump = list_remove(bloqueados_por_dump, 0);
		pthread_mutex_unlock(&m_bloqueados_por_dump);
		printf("TID HILO_DUMPO %d \n", hilo_dump->tid);
		if (rta == 0)
		{
			sem_post(&binario_corto_plazo);
			printf("RTA DEL BLOQUEAR 0 %d \n", rta);
			finalizar_tcb(hilo_dump);
		}
		else
		{
			sem_post(&binario_corto_plazo);
			printf("RTA DEL BLOQUEAR 1 %d \n", rta);
			agregar_a_ready_segun_alg(hilo_dump);
		}
		
		// hilo_en_ejecucion = NULL;
		
		break;

	case IO:
		char *cant_seg_duerme_ptr = list_get(instrucc->parametros, 0);
		int cant_seg_duerme = atoi(cant_seg_duerme_ptr);
		hilo = buscar_hilos_listas(hilo_en_ejecucion, hilo_en_ejecucion->tid);

		pthread_mutex_lock(&m_lista_io);
		list_add(lista_io, hilo);
		pthread_mutex_unlock(&m_lista_io);

		log_info(logger_kernel, "## (PID <%d> : TID <%d>) - Bloqueado por: <IO>",
				 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);

		//sem_post(&binario_corto_plazo);
		usleep(cant_seg_duerme);

		log_info(logger_kernel, "## ((PID <%d> : TID <%d> )) finalizó IO y pasa a READY",
				 hilo_en_ejecucion->pcb_padre_tcb->contador_tid, hilo_en_ejecucion->tid);
		hilo = list_remove(lista_io, 0);
		pasar_a_running_tcb_con_syscall(hilo);
		// agregar_a_ready_segun_alg(hilo);
		//free(hilo);
		break;
	}

	free(instrucc);
}

