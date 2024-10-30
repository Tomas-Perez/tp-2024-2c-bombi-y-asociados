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

void empaquetar_contexto(t_paquete *paquete, t_proceso *proceso, t_hilo *hilo)
{
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.PC, sizeof(uint32_t)); // ver si los registros van con "&"
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.AX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.BX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.CX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.DX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.EX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.FX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.GX, sizeof(uint32_t));
	agregar_a_paquete_solo(paquete, hilo->registros_hilo.HX, sizeof(uint32_t));
	// agregar_a_paquete_solo(paquete, proceso->base, sizeof(uint32_t));
	// agregar_a_paquete_solo(paquete, proceso->limite, sizeof(uint32_t));
}

t_registros_cpu recibir_contexto(t_registros_cpu registros, void *buffer)
{
	registros.PC = buffer_read_uint32(buffer);
	registros.AX = buffer_read_uint32(buffer);
	registros.BX = buffer_read_uint32(buffer);
	registros.CX = buffer_read_uint32(buffer);
	registros.DX = buffer_read_uint32(buffer);
	registros.EX = buffer_read_uint32(buffer);
	registros.FX = buffer_read_uint32(buffer);
	registros.GX = buffer_read_uint32(buffer);
	registros.HX = buffer_read_uint32(buffer);

	return registros;
}

void actualizar_contexto_en_memoria(t_proceso *proceso, t_hilo *hilo, t_registros_cpu registros)
{
	hilo->registros_hilo.PC = registros.PC;
	hilo->registros_hilo.AX = registros.AX;
	hilo->registros_hilo.BX = registros.BX;
	hilo->registros_hilo.CX = registros.CX;
	hilo->registros_hilo.DX = registros.DX;
	hilo->registros_hilo.EX = registros.EX;
	hilo->registros_hilo.FX = registros.FX;
	hilo->registros_hilo.GX = registros.GX;
	hilo->registros_hilo.HX = registros.HX;
	// proceso->base = base;
	// proceso->limite = limite;
}

void inicializar_particiones_fijas()
{
	char *particiones_char = eliminar_corchetes(particiones);
	char **vector_particiones = string_split(particiones_char, ","); // Separar por comas

	int base_sgte = 0;

	for (int i = 0; i < string_array_size(vector_particiones); i++)
	{
		t_particiones *particion = malloc(sizeof(t_particiones));

		if (particion == NULL)
		{ // Validar que malloc no falle
			perror("Error al asignar memoria para la partición");
			exit(EXIT_FAILURE);
		}

		particion->base = base_sgte;
		particion->limite = atoi(vector_particiones[i]);
		particion->ocupado = false;

		base_sgte += particion->limite;

		// Agregar la partición a la lista
		list_add(particiones_fijas, particion);
	}

	// Liberar la memoria del array de strings
	string_array_destroy(vector_particiones);
}

// FUNCIONES PARA INICIALIZAR MEMORIA

void inicializar_particiones_dinamicas()
{
	t_particiones *particion = malloc(sizeof(t_particiones));

	if (particion == NULL)
	{ // Validar que malloc no falle
		perror("Error al asignar memoria para la partición");
		exit(EXIT_FAILURE);
	}

	particion->base = 0;
	particion->limite = tamanio_memoria;
	particion->ocupado = false;

	list_add(particiones_dinamicas, particion);
}

char *eliminar_corchetes(char *cad)
{
	if (strlen(cad) <= 2)
	{
		return 0;
	}
	memmove(cad, cad + 1, strlen(cad));
	cad[strlen(cad) - 1] = '\0';
	return cad;
}
/*----------------------------------------------------PARTICIONES FIJAS----------------------------------------------------*/
// FUNCIONES PARA ASIGNAR HUECOS EN MEMORIA (PARTICIONES FIJAS)

t_particiones *asignar_first_fit_fijas(t_list *lista, uint32_t tamanio) // retorna partición o NULL si no hay hueco
{
	for (int i = 0; i < list_size(lista); i++)
	{
		t_particiones *particion = list_get(lista, i);
		if (particion->limite >= tamanio && particion->ocupado == 0)
		{
			particion->ocupado = 1;
			return particion;
		}
	}
	return NULL; // Retorna NULL si no hay hueco disponible
}

t_particiones *asignar_best_fit_fijas(t_list *lista, uint32_t tamanio)
{
	uint32_t mejor_particion = UINT32_MAX;
	uint32_t mejor_indice = UINT32_MAX;

	for (int i = 0; i < list_size(lista); i++)
	{
		t_particiones *particion = list_get(lista, i);
		if (particion->limite >= tamanio && particion->ocupado == 0 && particion->limite < mejor_particion)
		{
			mejor_particion = particion->limite;
			mejor_indice = i;
		}
	}

	if (mejor_indice != UINT32_MAX) // Si se encontró una partición adecuada
	{
		t_particiones *particion_a_devolver = list_get(lista, mejor_indice);
		particion_a_devolver->ocupado = 1;
		return particion_a_devolver;
	}

	return NULL; // Retorna NULL si no hay hueco adecuado
}

t_particiones *asignar_worst_fit_fijas(t_list *lista, uint32_t tamanio)
{
	list_sort(lista, particion_mayor); // Ordena la lista de mayor a menor segun tamaño de las particiones

	for (int i = 0; i < list_size(lista); i++)
	{
		t_particiones *particion = list_get(lista, i);
		if (particion->limite >= tamanio && particion->ocupado == 0)
		{
			particion->ocupado = 1;
			return particion;
		}
	}
	return NULL; // Retorna NULL si no hay hueco disponible
}

/*----------------------------------------------------FIN PARTICIONES FIJAS----------------------------------------------------*/

/*----------------------------------------------------PARTICIONES DINAMICAS----------------------------------------------------*/

t_particiones *asignar_first_fit_dinamicas(t_list *lista, uint32_t tamanio) // retorna partición o NULL si no hay hueco
{
	for (int i = 0; i < list_size(lista); i++)
	{
		t_particiones *particion = list_get(lista, i);
		if (particion->limite >= tamanio && particion->ocupado == 0)
		{
			particion->limite = tamanio;
			particion->ocupado = 1;

			// t_particiones *particion_anterior = list_get(lista, i-1);
			t_particiones *nueva_particion;
			nueva_particion->base = particion->base + particion->limite;
			if (i == 0) 
			{
				nueva_particion->limite = tamanio_memoria - tamanio;
				nueva_particion->ocupado = 0;
			} else {
				t_particiones *particion_anterior = list_get(lista, i-1);
				nueva_particion->limite = particion_anterior->limite - tamanio;
				nueva_particion->ocupado = 0;
			}
			list_add_in_index(lista, i+1, nueva_particion); // chequear
			return particion;
		}
	}
	return NULL; // Retorna NULL si no hay hueco disponible
}

t_particiones *asignar_best_fit_dinamicas(t_list *lista, uint32_t tamanio)
{
	
}

t_particiones *asignar_worst_fit_dinamicas(t_list *lista, uint32_t tamanio)
{
	list_sort(lista, particion_mayor); // Ordena la lista de mayor a menor segun tamaño de las particiones

	return NULL; // Retorna NULL si no hay hueco disponible
}



/*----------------------------------------------------FIN PARTICIONES DINAMICAS----------------------------------------------------*/
void liberar_espacio_memoria(t_proceso *proceso)
{
	t_particiones *particion_a_liberar = buscar_particion(particiones_fijas, proceso->base, proceso->limite);
	if (particion_a_liberar == NULL)
	{
		log_error(logger_memoria, "No se encontro la particion");
		exit(EXIT_FAILURE);
	}
	particion_a_liberar->ocupado = 0;
}

t_particiones *buscar_particion(t_list *lista, uint32_t base, uint32_t limite)
{

	for (int i = 0; i < list_size(lista); i++)
	{
		t_particiones *particion = list_get(lista, i);
		if (base == particion->base && limite == particion->limite)
		{
			return particion;
		}
	}
	return NULL;
}
bool particion_mayor(void *a, void *b)
{
	t_particiones *particion_a = (t_particiones *)a;
	t_particiones *particion_b = (t_particiones *)b;
	return particion_a->limite >= particion_b->limite;
}