// quizas no esta en la mejor carpeta
/*#include <stdio.h>
#include<stdlib.h>*/

#include "utilsKernel.h"
t_log* logger_kernel;
// --------------------------- Archivo inicial --------------------------- 

#define MAX_LENGTH 1000 // maximo tamaño de la cadena
#define MAX_LINES 10 

void abrir_e_interpretar_archivo_pseudocodigo(char* nombre_archivo)
{
        char* path = generar_path_archivo(nombre_archivo);
        interpretar_archivo_pseudocodigo(path);
        free(path);
        
}

char* generar_path_archivo(char* nombre_archivo)
{
        char *path_archivo = nombre_archivo;
        char *path_archivo_inicio = "/home/utnso/";
        char *path_archivo_completo = (char *)malloc(strlen(path_archivo_inicio) + strlen(path_archivo) + 1);
        strcpy(path_archivo_completo, path_archivo_inicio);
        strcat(path_archivo_completo, path_archivo);
        printf("%s\n", path_archivo_completo);
    
        return path_archivo_completo;
}

void interpretar_archivo_pseudocodigo(char *path_archivo)
{
    FILE* archivo;
    char *cadena;
    char linea[MAX_LENGTH];
    int contador = 0;
    t_comando* comando_archivo;
    comando_archivo = malloc(sizeof(t_comando));

    // creamos una lista y se la asignamos a parametros
    comando_archivo->parametros = list_create();

    
    archivo = fopen(path_archivo, "r");

    
    if (archivo == NULL)
    {
        printf("Error al abrir el archivo.\n");
        return -1;
    }

    cadena = (char *)malloc(MAX_LENGTH * MAX_LINES * sizeof(char));

    // Lee el archivo li­nea por linea y concatena las lineas en un solo string
    
    while (fgets(linea, MAX_LENGTH, archivo) != NULL && contador < MAX_LINES)
    {
        strcat(cadena, linea);
        contador++;
    }

    fclose(archivo);

  
    char **lineas = string_split(cadena, "\n");

    printf("Cantidad de comandos en el archivo: %d\n\n", contador);
    for (int i = 0; i < contador; i++)
    {
        
         printf("%s \n", lineas[i]);
        //interpretar_comando(comando_archivo, lineas[i]);       
        //acciones_archivo(comando_archivo);
        

        // Limpia los parámetros después de cada uso
        for (int i = 0; i < list_size(comando_archivo->parametros); i++)
        {
            char *parametro = list_remove(comando_archivo->parametros, i);
            free(parametro);
        }
        free(lineas[i]); // pensamos que liberar una a una las lineas nos iban a sol el problema
    }

    // Liberamos la mem
    for (int i = 0; i < list_size(comando_archivo->parametros); i++)
    {
        free(list_get(comando_archivo->parametros, i));
    }
    list_destroy(comando_archivo->parametros);
    free(comando_archivo);
    free(cadena);

    
}

// --------------------------- Inicializar ---------------------------
void inicializar_registros(pcb* proc)
{
        proc->registros_cpu.AX=0;
	    proc->registros_cpu.BX=0;
	    proc->registros_cpu.CX=0;
	    proc->registros_cpu.DX=0;
	    proc->registros_cpu.EX=0;
	    proc->registros_cpu.FX=0;
	    proc->registros_cpu.GX=0;
	    proc->registros_cpu.HX=0;
    	proc->registros_cpu.PC=0;

}

void inicializar_estructuras_kernel()
{
	int id_counter = 0;
    

	//mutex 
	pthread_mutex_init(&m_hilo_en_ejecucion,NULL);
	pthread_mutex_init(&m_proceso_en_ejecucion,NULL);
	pthread_mutex_init(&m_proceso_a_ejecutar,NULL);
	pthread_mutex_init(&m_lista_de_ready,NULL);
	pthread_mutex_init(&m_regreso_de_cpu,NULL);

	 //cola de procesos
	 lista_de_ready = list_create();
}
//  --------------------------- PCB  --------------------------- 

pcb *crear_pcb()
{
    pcb *nuevo_pcb = (pcb *)malloc(sizeof(pcb));
    nuevo_pcb->pid = id_counter;
    nuevo_pcb-> tid_counter = 0;
    
    id_counter++;
    nuevo_pcb->contador_tid = 0;

    inicializar_registros(nuevo_pcb);
    if (nuevo_pcb == NULL)
    {
        
        free(nuevo_pcb);
        return NULL;
    }
    return nuevo_pcb;
}

tcb* crear_tcb(pcb* proc_padre, int prioridad)
{
    tcb* nuevo_tcb;   
    nuevo_tcb->tid = proc_padre->contador_tid;
    nuevo_tcb->pid_padre_tcb = proc_padre->pid;
    nuevo_tcb->prioridad = prioridad;

    proc_padre->contador_tid++;
    //inicializar_registros();
    if (nuevo_tcb == NULL)
    {
        
        free(nuevo_tcb);
        return NULL;
    }
     return nuevo_tcb;
}


//  Planificador
void inicializar_hilos_planificacion()
{
   /* pthread_t hilo_plani_corto,hilo_plani_largo,hilo_exitt;

	pthread_create(&hilo_plani_corto, NULL,(void*) planificador_corto_plazo,NULL);
	pthread_create(&hilo_plani_largo,NULL,(void*) planificador_largo_plazo,NULL);
	pthread_create(&hilo_exitt,NULL, (void*) hilo_exit, NULL);

	pthread_detach(hilo_plani_corto);
	pthread_detach(hilo_plani_largo);
	//pthread_detach(hilo_exitt); */
}
// //  Planificador largo plazo


// //  Planificador corto plazo

