// quizas no esta en la mejor carpeta
/*#include <stdio.h>
#include<stdlib.h>*/

#include "utilsKernel.h"
t_log* logger_kernel;
// --------------------------- Archivo inicial --------------------------- 

#define MAX_LENGTH 1000 // maximo tamaño de la cadena
#define MAX_LINES 10 


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


// --------------------------- Inicializar ---------------------------

void inicializar_registros(tcb* hilo)
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
	pthread_mutex_init(&m_lista_de_ready,NULL);
	pthread_mutex_init(&m_regreso_de_cpu,NULL);
    pthread_mutex_init(&m_hilo_a_ejecutar, NULL); //TO DO destroy de los mutex

	 //cola de procesos
	 lista_de_ready = list_create();
}

void pedir_memoria(pcb* proceso_nuevo, int socket, int tamanio, char* path)
{

     
   
    //TO DO -> mili :) 
    // con el socket sale la conexion
    // mandar a memoria el motivo PROCESS_CREATE,
    // mandar en este orden

    // 1) el pid 2) el tam proc 3) tam path 4) el path, los chicos se van a encargar de verificar si hay suficiente
    // memoria para abrir el archivo y si no volvemos a 
    // hacer recv para confirmacion, seguro nos mandan un bool
    // el segundo parámetro es el tamaño del proceso en Memoria y 
    //el tercer parámetro es la prioridad del hilo main (TID 0)
    
        int pid = proceso_nuevo->pid;
        int motivo = PROCESS_CREATE; //preguntar si solo es para PROCESS CREATE entonces mandarle un nombre mas descriptivo


        t_paquete* pedido_memoria = crear_paquete(motivo);
		agregar_a_paquete(pedido_memoria, &pid,sizeof(int));
        agregar_a_paquete(pedido_memoria, &tamanio, sizeof(int));
        //agregar_a_paquete // ponemos el tam del path?
        agregar_a_paquete(pedido_memoria, path, strlen(path) + 1);// PATH PREGUNTAR

        enviar_paquete(pedido_memoria,socket);
		eliminar_paquete(pedido_memoria);

        int confirmacion = 0;
        recv(socket,&confirmacion, sizeof(int), MSG_WAITALL);

        if(confirmacion)
        {
            //agregar_a_ready(proceso_nuevo); TO DO
        }
        else
        {
            //esperar un SIGNAL de finalizo un proceso y llamar a la misma funcionn 
            //pedir_memoria(proceso_nuevo, socket, instruccion)
            // O
            // list_add(proceso_nuevo, bloqueados_por_mem_insuficiente); 
			// Y un hilo que chequee si esta vacia esta lista
           
        }
}
//  --------------------------- crear  --------------------------- 

pcb *crear_pcb(int prioridad_h_main)
{
    pcb* nuevo_pcb = (pcb *)malloc(sizeof(pcb));
    tcb* hilo_main;

    nuevo_pcb->contador_tid = 0;
    id_counter++;

    nuevo_pcb->lista_tcb = list_create();

    if (nuevo_pcb == NULL)
    {
        free(nuevo_pcb);
        return NULL;
    }
    hilo_main = crear_tcb(nuevo_pcb, prioridad_h_main);
    list_add(nuevo_pcb->lista_tcb, hilo_main);

    return nuevo_pcb;
}

tcb* crear_tcb(pcb* proc_padre, int prioridad)
{
    tcb* nuevo_tcb = (tcb *)malloc(sizeof(tcb));  
    nuevo_tcb->tid = proc_padre->contador_tid;
    nuevo_tcb->pid_padre_tcb = proc_padre->pid;
    nuevo_tcb->prioridad = prioridad;

    inicializar_registros(nuevo_tcb);
    proc_padre->contador_tid++;
    inicializar_registros(nuevo_tcb);

    if (nuevo_tcb == NULL)
    {
        
        free(nuevo_tcb);
        return NULL;
    }
     return nuevo_tcb;
}

pcb* crear_proceso_y_pedir_memoria(char* nombre_arch, int tam_proc, int prioridad, int socket)
 {  
    pcb *pcb_nuevo;

    char* path = generar_path_archivo(nombre_arch);
    pcb_nuevo = crear_pcb(prioridad); 
    pedir_memoria(pcb_nuevo, socket, tam_proc, path);

    return pcb_nuevo;
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
void recibir_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc){
		int cod_op = recibir_operacion(conexion_dispatch);
		if(cod_op == SYSCALL){
            desempaquetar_parametros_syscall_de_cpu(hilo, motivo, instrucc);
			printf("TID: %i, motivo: %i", hilo->tid, *motivo);
            
		}
		else {printf("Codigo de Operacion de CPU incorrecto\n");
               
        }
}

void desempaquetar_parametros_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc){
		int tam;
		void* buffer = recibir_buffer(&tam, conexion_dispatch);
		int desplazamiento = 0;
		/*memcpy(&(proc->program_counter), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento+= sizeof(int)*/;
		memcpy(motivo, buffer + desplazamiento, sizeof(int));
		desplazamiento+= sizeof(int);
		printf("TID 2: %i, motivo 2: %i", hilo->tid, *motivo);
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




// --------------------- finalizar ---------------------

void finalizar_hilos_proceso(int pid)
{
    pcb* proceso = buscar_proc_lista(lista_de_ready,pid);
    tcb* hilo_a_finlizar;
    while(list_is_empty(proceso->lista_tcb) == 0)
    {
        hilo_a_finlizar = list_remove(proceso->lista_tcb,0);
        finalizar_tcb(hilo_a_finlizar);
    }
}

void finalizar_tcb(tcb* hilo_a_finlizar)
{
    // TO DO
}

// --------------------- buscar ---------------------
pcb *buscar_proc_lista(t_list *lista, int pid_buscado)
{
	int elementos = list_size(lista);
	for (int i = 0; i < elementos; i++)
	{
		pcb *pcb = list_get(lista, i);
		if (pid_buscado == pcb->pid)
		{
			return pcb;
		}
	}
	return NULL;
}