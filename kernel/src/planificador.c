#include "planificador.h"
int id_counter;

tcb* hilo_en_ejecucion; 
pcb* proceso_en_ejecucion; //VER SI ESTA BIEN ACA 
pthread_mutex_t m_hilo_en_ejecucion;
pthread_mutex_t m_proceso_en_ejecucion;
pthread_mutex_t m_proceso_a_ejecutar;
pthread_mutex_t m_cola_de_ready;
pthread_mutex_t m_regreso_de_cpu;
t_queue* cola_de_ready;


void pasar_a_running_tcb(tcb* tcb_listo)
{
   // mandar_tcb_dispatch(tcb_listo);
    pthread_mutex_lock(&m_hilo_en_ejecucion);
	hilo_en_ejecucion = tcb_listo; 
	pthread_mutex_unlock(&m_hilo_en_ejecucion);
    log_info(logger_kernel, "PID <%d> TID: <%d> - Estado Anterior: READY - Estado Actual: EXEC",
    proceso_en_ejecucion->pid, tcb_listo->tid);
}

 void pasar_a_running_pcb(pcb* proceso_listo){
		
		pthread_mutex_lock(&m_proceso_en_ejecucion);
		proceso_en_ejecucion = proceso_listo; //OJO posible causa de error
		pthread_mutex_unlock(&m_proceso_en_ejecucion);
        log_info(logger_kernel, "PID: <%d> - Estado AnterSior: READY - Estado Actual: EXEC", proceso_listo->pid);

        // aca llamar a planificador corto plazo que llame a pasar_a_running_tcb
}

void planificador_corto_plazo_pcb()
{
    while(1)
    {
		pcb* proceso_a_ejecutar;
        //sem_wait(&binario_corto_plazo); VER COMO LLAMAMOS SEMAFOROS
	  	//sem_wait(&cont_procesos_en_ready);

        if (queue_is_empty(cola_de_ready)) 
        {

      		log_error(logger_kernel, "Cola de ready está vacía en planificador_corto_plazo\n");
        	pthread_mutex_unlock(&m_cola_de_ready);
			return 1;
		}
        
        pthread_mutex_lock(&m_proceso_a_ejecutar);
		pthread_mutex_lock(&m_cola_de_ready);
		proceso_a_ejecutar = queue_pop(cola_de_ready);
		pthread_mutex_unlock(&m_cola_de_ready);
        //pthread_mutex_unlock(&m_proceso_a_ejecutar);
                
				
        if(proceso_a_ejecutar == NULL)
        {
            log_error(logger_kernel,"Proceso a ejecutar es NULL en planificador_corto_plazo\n");
       	    return 1;
    	}	
        
        pthread_mutex_lock(&m_regreso_de_cpu);
		//proceso_a_ejecutar->regreso_de_cpu = 0; VER: seguimos usando esto?
		pthread_mutex_unlock(&m_regreso_de_cpu);

		pasar_a_running_pcb(proceso_a_ejecutar);
		pthread_mutex_unlock(&m_proceso_a_ejecutar);
		
        //solicitud_de_cpu();				
    }
}