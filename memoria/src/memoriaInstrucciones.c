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

