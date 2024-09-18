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
	pthread_mutex_init(&m_instruccion,NULL);
	pthread_mutex_init(&m_proc_mem,NULL);
}

t_proceso *agregar_proceso_instrucciones(FILE *f, int pid) // habria que mandar por parametro el archivo que nos mandan desde kernel
{
	char *instruccion = NULL;
	size_t longitud = 0;
	int cant_instrucciones = 0;

	t_proceso *proceso = malloc(sizeof(t_proceso)); // reservamos espacio en memoria para el proceso
	proceso->pid = pid;
	proceso->tids = list_create();
    list_add(proceso->tids, 0); // Agrega el hilo 0 pq es el hilo main
    // Ver como agregar base y limite

    t_hilo *hilo_padre = malloc(sizeof(t_hilo));
    hilo_padre->tid = 0;
    hilo_padre->pid_padre = pid;
    hilo_padre->instrucciones = list_create();
    //hilo_padre->registros_hilo = inicializar_registros(); // HACER FUNCION PARA INICIALIZAR REGISTROS

	while (getline(&instruccion, &longitud, f) != -1) // Se leen las instrucciones del archivo línea por línea
	{
		if (strcmp(instruccion, "\n")) // Verifica si la línea no es un salto de línea vacío
		{
			int longitud = strlen(instruccion);
			if (instruccion[longitud - 1] == '\n') // Si la línea termina con '\n', lo reemplaza con '\0'
				instruccion[longitud - 1] = '\0';

			char *aux = malloc(strlen(instruccion) + 1); // Asigna memoria suficiente para la instrucción
			strcpy(aux, instruccion);					 // Copia la instrucción en la memoria asignada

			pthread_mutex_lock(&mutex_instrucciones);
			list_add(hilo_padre->instrucciones, aux); // Añade la instrucción a la lista
			pthread_mutex_unlock(&mutex_instrucciones);

			cant_instrucciones++;
		}
	}
	free(instruccion);
	fclose(f);
	for (int i = 0; i < list_size(hilo_padre->instrucciones); i++)
	{
		char *aux = list_get(hilo_padre->instrucciones, i);
		printf("%s\n", aux); // muestra las instrucciones linea por linea
	}
	pthread_mutex_lock(&mutex_listas);
	list_add(procesos_memoria, proceso); // Agrega proceso a lista de procesos
	pthread_mutex_unlock(&mutex_listas);

	return proceso;
}
