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
void inicializar_registros_tcb(tcb* hilo)
{
        hilo->registros_cpu.AX=0;
	    hilo->registros_cpu.BX=0;
	    hilo->registros_cpu.CX=0;
	    hilo->registros_cpu.DX=0;
	    hilo->registros_cpu.EX=0;
	    hilo->registros_cpu.FX=0;
	    hilo->registros_cpu.GX=0;
	    hilo->registros_cpu.HX=0;
    	hilo->registros_cpu.PC=0;

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

void pedir_memoria(pcb* proceso_nuevo, int socket)
{
    //TO DO -> mili :) 
    // con el socket sale la conexion
    // mandar a memoria el motivo PROCESS_CREATE,
    // mandar en este orden
    // 1) el pid 2) el tam proc 3) tam path 4) el path, los chicos se van a encargar de verificar si hay suficiente
    // memoria para abrir el archivo y si no volvemos a 
    // hacer recv para confirmacion, seguro nos mandan un bool

        if(proceso_nuevo->mem_asignada == 1)
        {
            //agregar_a_ready(proceso_nuevo);
        }
        else
        {
            // list_add(proceso_nuevo, bloqueados_por_mem_insuficiente); NO
			// 
            // habria que preguntar si se quiere crear otro proc nuevo para el que si hay suficiente memoria
            // hay que ponerlo igual en la lista de bloqueados x mem insuficiente
            // aca vendria la funcion recursiva (¿recursiva? creo q si para que se llame hasta q se cumpla el caso
            // base(que la memoria sea suficiente))
        }
}
//  --------------------------- PCB  --------------------------- 

pcb *crear_pcb(int prioridad_h_main)
{
    pcb* nuevo_pcb = (pcb *)malloc(sizeof(pcb));
    tcb* hilo_main;
    nuevo_pcb->pid = id_counter;
    nuevo_pcb->contador_tid = 0;
    // lista de los tids 
    nuevo_pcb->tid = 0;
    id_counter++;
    nuevo_pcb->contador_tid = 0;
    nuevo_pcb->lista_tcb = list_create();

   // inicializar_registros(nuevo_pcb); //PREGUNTAR
    if (nuevo_pcb == NULL)
    {
        free(nuevo_pcb);
        return NULL;
    }
    hilo_main=crear_tcb(nuevo_pcb, prioridad_h_main);
    list_add(nuevo_pcb->lista_tcb, hilo_main);

    return nuevo_pcb;
}

tcb* crear_tcb(pcb* proc_padre, int prioridad)
{
    tcb* nuevo_tcb = (tcb *)malloc(sizeof(tcb));;   
    nuevo_tcb->tid = proc_padre->contador_tid;
    nuevo_tcb->pid_padre_tcb = proc_padre->pid;
    nuevo_tcb->prioridad = prioridad;
    inicializar_registros_tcb(nuevo_tcb);
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

// Parametros
void recibir_syscall_de_cpu(pcb* proc, int* motivo, instruccion* instrucc){
		int cod_op = recibir_operacion(conexion_dispatch);
		if(cod_op == SYSCALL){
			desempaquetar_parametros_syscall_de_cpu(proc, motivo, instrucc);
			printf("PID: %i, motivo: %i", proc->pid, *motivo);
		}
		else printf("Codigo de Operacion de CPU incorrecto\n");
}

void desempaquetar_parametros_syscall_de_cpu(pcb* proc, int* motivo, instruccion* instrucc){
		int tam;
		void* buffer = recibir_buffer(&tam, conexion_dispatch);
		int desplazamiento = 0;

		/*memcpy(&(proc->program_counter), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento+= sizeof(int)*/;
		memcpy(motivo, buffer + desplazamiento, sizeof(int));
		desplazamiento+= sizeof(int);
		printf("PID 2: %i, motivo 2: %i", proc->pid, *motivo);
		memcpy(&(instrucc->cant_parametros), buffer + desplazamiento, sizeof(int));
		desplazamiento+= sizeof(int);
		for(int i = 0; i < instrucc->cant_parametros; i++){

			char* parametro; 
			int tamanio_parametro;
			memcpy(&(tamanio_parametro), buffer + desplazamiento, sizeof(int));
			desplazamiento+= sizeof(int);
			parametro= (char*) calloc(1,sizeof(tamanio_parametro));
			//parametro=malloc(size_parametro);
			if (parametro == NULL) 
            {
                // Manejar el error de memoria
                log_error(logger_kernel, "Error al asignar memoria para el parámetro");
                // Liberar recursos y salir de la función
                free(buffer);
                return;
            }
			memcpy(parametro, buffer + desplazamiento, tamanio_parametro);
			desplazamiento+= tamanio_parametro;

			list_add(instrucc->parametros, parametro);
		}

		free(buffer);
	}


