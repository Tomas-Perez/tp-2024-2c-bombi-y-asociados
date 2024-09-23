#include "memoriaInstrucciones.h"

t_list *procesos_memoria;

pthread_mutex_t mutex_listas;
pthread_mutex_t mutex_instrucciones;
pthread_mutex_t m_instruccion;
pthread_mutex_t m_proc_mem;

void inicializar_estructuras()
{
	procesos_memoria = list_create();
	pthread_mutex_init(&mutex_listas, NULL);
	pthread_mutex_init(&mutex_instrucciones, NULL);
	pthread_mutex_init(&m_instruccion, NULL);
	pthread_mutex_init(&m_proc_mem, NULL);
}

t_proceso *agregar_proceso_instrucciones(FILE *f, int pid) // habria que mandar por parametro el archivo que nos mandan desde kernel
{
	t_proceso *proceso = malloc(sizeof(t_proceso)); // reservamos espacio en memoria para el proceso
	proceso->pid = pid;
	proceso->tids = list_create();

	// Ver como agregar base y limite

	t_hilo *hilo_main = malloc(sizeof(t_hilo));
	inicializar_hilo(proceso, 0, hilo_main, f);
	list_add(proceso->tids, hilo_main); // Agrego hilo main a lista de hilos dentro de procesos

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

// FUNCIONES PARA MANDAR A UTILS

void empaquetar_contexto(t_paquete *paquete, t_proceso *proceso, t_hilo *hilo)
{
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.PC, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.AX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.BX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.CX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.DX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.EX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.FX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.GX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.HX, sizeof(uint32_t));
	//agregar_a_paquete_solo(paquete, proceso->base, sizeof(uint32_t));
	//agregar_a_paquete_solo(paquete, proceso->limite, sizeof(uint32_t));
}