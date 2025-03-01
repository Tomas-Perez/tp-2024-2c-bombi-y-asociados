#include "planificador.h"
int id_counter;
int indice;

int quantum_restante;
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
pthread_mutex_t m_quantum_restante;
pthread_mutex_t m_syscall_solicitada;
pthread_mutex_t m_lista_io;
pthread_mutex_t m_bloqueados_por_dump;
pthread_mutex_t m_syscall_replanificadora;
pthread_mutex_t m_contador;

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
sem_t bin_dispatch;
sem_t bin_memoria;

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
		printf("Se agrego el hilo PID <%d> : TID <%d> a la cola multinivel\n", hilo->pcb_padre_tcb->pid, hilo->tid);
		pthread_mutex_unlock(&(cola_nivel->m_lista_prioridad));
	}
	sem_post(&hilos_en_ready);
}

void planificador_corto_plazo()
{
	while (1)
	{
		sem_wait(&binario_corto_plazo);
		pthread_mutex_lock(&m_contador);
		while (contador != 0)
		{
			pthread_mutex_unlock(&m_contador);
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
				pasar_a_running_tcb(hilo_a_ejecutar);
				atender_syscall();
			}
			if (strcmp(algoritmo_de_planificacion, "PRIORIDADES") == 0)
			{
				verificar_lista_ready(lista_de_ready);
				pasar_a_running_tcb_prioridades();
				atender_syscall();
			}
			if (strcmp(algoritmo_de_planificacion, "CMN") == 0)
			{
				pthread_mutex_lock(&m_lista_multinivel);
				if (list_size(lista_multinivel) > 0)
				{
					pthread_mutex_unlock(&m_lista_multinivel);

					nivel_prioridad *mayor_nivel;
					mayor_nivel = encontrar_nivel_mas_prioritario(lista_multinivel);
					if (mayor_nivel != NULL)
					{
						if (list_is_empty(mayor_nivel->hilos_asociados) == false)
						{
							hilo_a_ejecutar = list_remove(mayor_nivel->hilos_asociados, 0);
							if (list_size(mayor_nivel->hilos_asociados) == 0)
							{
								bool loBorra = list_remove_element(lista_multinivel, mayor_nivel);
								list_destroy(mayor_nivel->hilos_asociados);
								pthread_mutex_destroy(&(mayor_nivel->m_lista_prioridad));
								free(mayor_nivel);
							}
						}

						pthread_t tround_robin;

						pasar_a_running_tcb(hilo_a_ejecutar);
						pthread_create(&tround_robin, NULL, (void *)desalojar_por_RR, (void *)hilo_a_ejecutar);

						atender_syscall();
						pthread_detach(tround_robin);
					}
				}
				else
				{
					pthread_mutex_unlock(&m_lista_multinivel);
					printf("No hay hilos en ready.\n");
				}
			}
		}
		pthread_mutex_unlock(&m_contador);
		pthread_exit(NULL);
	}
}

void pasar_a_running_tcb(tcb *tcb_listo)
{
	pthread_mutex_lock(&m_syscall_replanificadora);
	syscall_replanificadora = 0;
	pthread_mutex_unlock(&m_syscall_replanificadora);

	mandar_tcb_dispatch(tcb_listo);
	pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo;
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
	pthread_mutex_lock(&m_quantum_restante);
	quantum_restante = 1;
	pthread_mutex_unlock(&m_quantum_restante);
	log_info(logger_kernel, "(PID <%d> TID: <%d>) - Estado Anterior: READY - Estado Actual: EXEC",
			 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
}

void pasar_a_running_tcb_con_syscall(tcb *tcb_listo)
{
	pthread_mutex_lock(&m_quantum_restante);
	if (quantum_restante == 1)
	{
		pthread_mutex_unlock(&m_quantum_restante);
		mandar_tcb_dispatch(tcb_listo);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		hilo_en_ejecucion = tcb_listo;
		pthread_mutex_unlock(&m_hilo_en_ejecucion);
		atender_syscall();
	}
	else
	{
		pthread_mutex_unlock(&m_quantum_restante);
		agregar_a_ready_multinivel(tcb_listo);
		sem_post(&binario_corto_plazo);
	}
}

void pasar_a_running_tcb_prioridades()
{

	tcb *tcb_listo = elegir_segun_prioridades();
	pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo;
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
	pthread_mutex_lock(&m_syscall_replanificadora);
	syscall_replanificadora = 0;
	pthread_mutex_unlock(&m_syscall_replanificadora);
	mandar_tcb_dispatch(tcb_listo);
	log_info(logger_kernel, "PID <%d> TID: <%d>  - Estado Anterior: READY - Estado Actual: EXEC",
			 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);
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

	log_info(logger_kernel, "## (PID <%d> : TID <%d> ) - Solicitó syscall: <%s>",
			 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid, op_code_to_string(motivo));
	pthread_mutex_lock(&m_syscall_replanificadora);
	syscall_replanificadora = 1;
	pthread_mutex_unlock(&m_syscall_replanificadora);

	switch (motivo)
	{
	case RR:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		agregar_a_ready_multinivel(hilo_en_ejecucion);
		pthread_mutex_unlock(&m_hilo_en_ejecucion);
		sem_post(&binario_corto_plazo);
		break;
	case PROCESS_CREATE:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);

		archivo = list_get(instrucc->parametros, 0);
		char *tamanio = list_get(instrucc->parametros, 1);
		char *prioridad_PC = list_get(instrucc->parametros, 2);
		int tam = atoi(tamanio);
		int priori = atoi(prioridad_PC);
		sem_wait(&bin_memoria);
		int socket_PC = conectarMemoria();
		pcb *proceso_nuevo = crear_pcb(priori, archivo, tam, socket_PC);

		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);

		break;
	case SEGMENTATION_FAULT:
	case PROCESS_EXIT:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		if (hilo_en_ejecucion->tid == 0)
		{
			pthread_mutex_unlock(&m_hilo_en_ejecucion);
			pthread_mutex_lock(&m_syscall_replanificadora);
			syscall_replanificadora = 1;
			pthread_mutex_unlock(&m_syscall_replanificadora);
			finalizar_proceso(hilo_en_ejecucion->pcb_padre_tcb);
		}
		pthread_mutex_unlock(&m_hilo_en_ejecucion);
		break;

	case THREAD_CREATE:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);

		archivo = list_get(instrucc->parametros, 0);
		prioridad = atoi(list_get(instrucc->parametros, 1));
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		pcb *proceso = hilo_en_ejecucion->pcb_padre_tcb;
		pthread_mutex_unlock(&m_hilo_en_ejecucion);

		sem_wait(&bin_memoria);
		socket = conectarMemoria();
		tcb *hilo = crear_tcb(proceso, prioridad);
		iniciar_hilo(hilo, socket, archivo);
		close(socket);
		sem_post(&bin_memoria);
		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);

		break;
	case THREAD_JOIN:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);
		char *tid_j = list_get(instrucc->parametros, 0);
		tid = atoi(tid_j);
		tcb *tcb_invocado = buscar_tid(hilo_en_ejecucion->pcb_padre_tcb->lista_tcb, tid);

		if (tcb_invocado != NULL)
		{

			pthread_mutex_lock(&m_syscall_replanificadora);
			syscall_replanificadora = 1;
			pthread_mutex_unlock(&m_syscall_replanificadora);
			buscar_hilos_listas_sin_sacar_del_padre(hilo_en_ejecucion, hilo_en_ejecucion->tid);

			log_info(logger_kernel, "## (PID <%d> : TID <%d>) - Bloqueado por: <PTHREAD_JOIN>",
					 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);

			list_add(tcb_invocado->block_join, hilo_en_ejecucion);

			sem_post(&binario_corto_plazo);
		}
		else
		{
			log_info(logger_kernel, "No se encontró el hilo");
			pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		}
		break;

	case THREAD_CANCEL:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);
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
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 1;
		pthread_mutex_unlock(&m_syscall_replanificadora);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		tcb *hilo_exit_aux = hilo_en_ejecucion;
		pthread_mutex_unlock(&m_hilo_en_ejecucion);
		finalizar_tcb(hilo_exit_aux);
		sem_post(&binario_corto_plazo);
		break;
	case MUTEX_CREATE:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);

		mutex_k *nuevo_mutex;
		char *nombre_mutex = list_get(instrucc->parametros, 0);
		nuevo_mutex = crear_mutex(nombre_mutex);
		log_info(logger_kernel, "## Se crea el MUTEX: <%s>", nuevo_mutex->nombre);
		pthread_mutex_lock(&m_hilo_en_ejecucion);
		list_add(hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc, nuevo_mutex);
		pthread_mutex_unlock(&m_hilo_en_ejecucion);

		pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		break;
	case MUTEX_LOCK:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);

		char *nombre_mutex_solic;
		nombre_mutex_solic = list_get(instrucc->parametros, 0);

		pthread_mutex_lock(&m_hilo_en_ejecucion);
		lista_mutex_proceso = hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc;
		pthread_mutex_unlock(&m_hilo_en_ejecucion);

		if (list_size(lista_mutex_proceso) > 0)
		{
			mutex_k *aux = existe_mutex_por_nombre(nombre_mutex_solic, lista_mutex_proceso);
			if (aux != NULL)
			{
				if (aux->disponibilidad == true)
				{
					asignar_mutex_hilo(aux, hilo_en_ejecucion);
					pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
				}
				else
				{
					pthread_mutex_lock(&m_syscall_replanificadora);
					syscall_replanificadora = 1;
					pthread_mutex_unlock(&m_syscall_replanificadora);
					buscar_hilos_listas_sin_sacar_del_padre(hilo_en_ejecucion, hilo_en_ejecucion->tid);
					list_add(aux->bloqueados_por_mutex, hilo_en_ejecucion);

					log_info(logger_kernel, "## (PID <%d> : TID <%d>) - Bloqueado por: <MUTEX: %s>",
							 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid, aux->nombre);

					sem_post(&binario_corto_plazo);
				}
			}
			else
			{
				pthread_mutex_lock(&m_syscall_replanificadora);
				syscall_replanificadora = 1;
				pthread_mutex_unlock(&m_syscall_replanificadora);
				finalizar_tcb(hilo_en_ejecucion);
				sem_post(&binario_corto_plazo);
			}
		}
		else
		{
			pthread_mutex_lock(&m_syscall_replanificadora);
			syscall_replanificadora = 1;
			pthread_mutex_unlock(&m_syscall_replanificadora);

			finalizar_tcb(hilo_en_ejecucion);
			sem_post(&binario_corto_plazo);
		}
		break;
	case MUTEX_UNLOCK:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 0;
		pthread_mutex_unlock(&m_syscall_replanificadora);

		char *nombre_mutex_solici;
		nombre_mutex_solici = list_get(instrucc->parametros, 0);

		lista_mutex_proceso = hilo_en_ejecucion->pcb_padre_tcb->lista_mutex_proc;
		mutex_k *aux_m = existe_mutex_por_nombre(nombre_mutex_solici, lista_mutex_proceso);
		log_info(logger_kernel, "## Se libera el MUTEX: <%s> ", aux_m->nombre);
		if (aux_m != NULL)
		{
			bool esTomado = mutex_por_nombre_tomado_por_hilo(nombre_mutex_solici, hilo_en_ejecucion);
			if (esTomado != false)
			{
				list_remove_element(hilo_en_ejecucion->lista_mutex, aux_m);
				pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
				aux_m->disponibilidad = true;
				aux_m->hilo_poseedor = NULL;
				asignar_mutex_al_primer_bloqueado(aux_m);
			}
		}
		else
		{
			pasar_a_running_tcb_con_syscall(hilo_en_ejecucion);
		}
		break;
	case DUMP_MEMORY:
		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 1;
		pthread_mutex_unlock(&m_syscall_replanificadora);

		pthread_mutex_lock(&m_hilo_en_ejecucion);
		tcb *hilo_dumpo = hilo_en_ejecucion;
		pthread_mutex_unlock(&m_hilo_en_ejecucion);
		sem_wait(&bin_memoria);
		socket = conectarMemoria();
		bloquear_por_dump(hilo_dumpo, socket);

		paquete_dump *dump_o = malloc(sizeof(paquete_dump));
		dump_o->hilo = hilo_dumpo;
		dump_o->socket = socket;

		pthread_t hilo_dump;
		pthread_create(&hilo_dump, NULL, (void *)hacerDump, dump_o);
		pthread_detach(hilo_dump);

		sem_post(&binario_corto_plazo);

		break;

	case IO:

		pthread_mutex_lock(&m_syscall_replanificadora);
		syscall_replanificadora = 1;
		pthread_mutex_unlock(&m_syscall_replanificadora);

		char *cant_seg_duerme_ptr = list_get(instrucc->parametros, 0);
		int cant_seg_duerme = atoi(cant_seg_duerme_ptr);
		hilo = buscar_tid(hilo_en_ejecucion->pcb_padre_tcb->lista_tcb, hilo_en_ejecucion->tid);
		if (hilo != NULL)
		{

			pthread_mutex_lock(&m_lista_io);
			list_add(lista_io, hilo);
			pthread_mutex_unlock(&m_lista_io);

			log_info(logger_kernel, "## (PID <%d> : TID <%d>) - Bloqueado por: <IO>",
					 hilo_en_ejecucion->pcb_padre_tcb->pid, hilo_en_ejecucion->tid);

			sem_post(&binario_corto_plazo);

			pthread_t hilo_io;
			pthread_create(&hilo_io, NULL, (void *)hacerIO, cant_seg_duerme);
			pthread_detach(hilo_io);
		}
		break;
	}

	liberar_param_instruccion(instrucc);
}

void *hacerIO(int cant_seg_duerme)
{
	usleep(cant_seg_duerme * 1000);

	pthread_mutex_lock(&m_lista_io);
	tcb *hilo_aux = list_remove(lista_io, 0);
	pthread_mutex_unlock(&m_lista_io);

	log_info(logger_kernel, "## ((PID <%d> : TID <%d> )) finalizó IO y pasa a READY",
			 hilo_aux->pcb_padre_tcb->pid, hilo_aux->tid);
	agregar_a_ready_segun_alg(hilo_aux);
}

void *hacerDump(paquete_dump *info_dump)
{
	int finalizo_operacion;
	recv(info_dump->socket, &finalizo_operacion, sizeof(int), MSG_WAITALL);
	close(info_dump->socket);
	sem_post(&bin_memoria);

	pthread_mutex_lock(&m_bloqueados_por_dump);
	tcb *dump = buscar_hilo_dump(info_dump);
	pthread_mutex_unlock(&m_bloqueados_por_dump);


	if (finalizo_operacion == 0)
	{
		finalizar_proceso(dump->pcb_padre_tcb);
	}
	else
	{
		agregar_a_ready_segun_alg(dump);
	}
	free(info_dump);
}

tcb *buscar_hilo_dump(paquete_dump *info_dump)
{
	for (int i = 0; i < list_size(bloqueados_por_dump); i++)
	{
		tcb *aux = list_get(bloqueados_por_dump, i);
		if (aux->tid == info_dump->hilo->tid && aux->pcb_padre_tcb->pid == info_dump->hilo->pcb_padre_tcb->pid)
		{
			list_remove(bloqueados_por_dump, i);
			return aux;
		}
	}
	return NULL;
}
