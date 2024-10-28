#include "memoriaInstrucciones.h"

t_list *procesos_memoria;
t_list *particiones_fijas;
t_list *particiones_dinamicas;

pthread_mutex_t mutex_listas;
pthread_mutex_t mutex_instrucciones;
pthread_mutex_t mutex_tids;
pthread_mutex_t m_instruccion;
pthread_mutex_t m_proc_mem;

void inicializar_estructuras()
{
	procesos_memoria = list_create();
	particiones_fijas = list_create();
	particiones_dinamicas = list_create();
	pthread_mutex_init(&mutex_listas, NULL);
	pthread_mutex_init(&mutex_instrucciones, NULL);
	pthread_mutex_init(&m_instruccion, NULL);
	pthread_mutex_init(&m_proc_mem, NULL);
}

t_proceso *agregar_proceso_instrucciones(FILE *f, int pid, t_particiones *particion_a_asignar) // habria que mandar por parametro el archivo que nos mandan desde kernel
{
	t_proceso *proceso = malloc(sizeof(t_proceso)); // reservamos espacio en memoria para el proceso
	
	proceso->pid = pid;
	proceso->tids = list_create();
	proceso->base = particion_a_asignar->base;
	proceso->limite = particion_a_asignar->limite;

	t_hilo *hilo_main = malloc(sizeof(t_hilo));
	inicializar_hilo(proceso, 0, hilo_main, f); // Ver si agregar prioridad
	list_add(proceso->tids, hilo_main);			// Agrego hilo main a lista de hilos dentro de procesos

	pthread_mutex_lock(&mutex_listas);
	list_add(procesos_memoria, proceso); // Agrega proceso a lista de procesos
	pthread_mutex_unlock(&mutex_listas);

	return proceso;
}

char *buscar_instruccion(int pid, int tid, uint32_t program_counter)
{
	t_proceso *proceso = buscar_proceso(procesos_memoria, pid);
	if (proceso != NULL)
	{
		t_hilo *hilo = buscar_hilo(proceso, tid);
		if (hilo != NULL)
		{
			pthread_mutex_lock(&mutex_instrucciones);
			char *instruccion = list_get(hilo->instrucciones, program_counter); // devuelve instruccion de la lista segun PC
			pthread_mutex_unlock(&mutex_instrucciones);
			printf(" %s\n", instruccion);
			return instruccion;
		}
		printf("NO ENCONTRE EL HILO");
	}
	printf("NO ENCONTRE EL PROCESO");
	return NULL;
}

t_proceso *buscar_proceso(t_list *lista, int pid_buscado)
{
	int elementos = list_size(lista);
	for (int i = 0; i < elementos; i++)
	{
		t_proceso *proceso = list_get(lista, i);
		if (pid_buscado == proceso->pid)
		{
			return proceso;
		}
	}
	printf("Proceso no encontrado\n");
	// list_destroy(lista);
	return NULL;
}

t_hilo *buscar_hilo(t_proceso *proceso_padre, int tid_buscado)
{
	int elementos = list_size(proceso_padre->tids);
	for (int i = 0; i < elementos; i++)
	{
		t_hilo *hilo = list_get(proceso_padre->tids, i);
		if (tid_buscado == hilo->tid)
		{
			return hilo;
		}
	}
	printf("Hilo no encontrado\n");
	return NULL;
}

// Funciones para eliminar estructuras

void eliminar_proceso(int pid)
{
	t_proceso *proceso_a_eliminar = buscar_proceso(procesos_memoria, pid);
	pthread_mutex_lock(&m_proc_mem);
	list_remove_element(procesos_memoria, proceso_a_eliminar);
	pthread_mutex_unlock(&m_proc_mem);
	liberar_proceso(proceso_a_eliminar);
}

void liberar_proceso(t_proceso *proceso)
{
	for (int i = 0; i < list_size(proceso->tids); i++)
	{
		pthread_mutex_lock(&mutex_tids);
		free(list_get(proceso->tids, i));
		pthread_mutex_unlock(&mutex_tids);
	}
	free(proceso);
}

void eliminar_hilo(int pid, int tid)
{
	t_proceso *proceso_padre = buscar_proceso(procesos_memoria, pid);
	t_hilo *hilo_a_eliminar = buscar_hilo(proceso_padre, tid);
	liberar_hilo(hilo_a_eliminar);
}


void liberar_hilo(t_hilo *hilo)
{
	for (int i = 0; i < list_size(hilo->instrucciones); i++)
	{
		pthread_mutex_lock(&mutex_instrucciones);
		free(list_get(hilo->instrucciones, i));
		pthread_mutex_unlock(&mutex_instrucciones);
	}
	list_destroy(hilo->instrucciones);
	free(hilo);
}