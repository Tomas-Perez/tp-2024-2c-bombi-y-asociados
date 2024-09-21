#include "utilsMemoria.h"

void inicializar_hilo(t_proceso *proceso_padre, int tid, t_hilo *hilo_a_inicializar, FILE *f)
{
	hilo_a_inicializar->tid = tid;
	hilo_a_inicializar->pid_padre = proceso_padre->pid; // nose si hace falta
	inicializar_registros(hilo_a_inicializar);
	guardar_instrucciones(hilo_a_inicializar, f);
}

void inicializar_registros(t_hilo *hilo)
{
	hilo->registros_hilo.AX = 0;
	hilo->registros_hilo.BX = 0;
	hilo->registros_hilo.CX = 0;
	hilo->registros_hilo.DX = 0;
	hilo->registros_hilo.EX = 0;
	hilo->registros_hilo.FX = 0;
	hilo->registros_hilo.PC = 0;
}

void guardar_instrucciones(t_hilo *hilo, FILE *f)
{
	char *instruccion = NULL;
	size_t longitud = 0;

	hilo->instrucciones = list_create();

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
			list_add(hilo->instrucciones, aux); // Añade la instrucción a la lista
			pthread_mutex_unlock(&mutex_instrucciones);
		}
	}
	free(instruccion);
	fclose(f);
	for (int i = 0; i < list_size(hilo->instrucciones); i++)
	{
		char *aux = list_get(hilo->instrucciones, i);
		printf("%s\n", aux); // muestra las instrucciones linea por linea
	}
}