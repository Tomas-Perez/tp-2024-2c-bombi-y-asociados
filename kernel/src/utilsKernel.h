#include <utils/utils.h>
char* generar_path_archivo(char* nombre_archivo);
pcb *crear_pcb(int prioridad, char* path, int tamanio);
tcb* crear_tcb(pcb* proc_padre, int prioridad);
void inicializar_registros(tcb* hilos);
void finalizar_hilos_proceso(pcb* proceso);
pcb *buscar_proc_lista(t_list *lista, int pid_buscado);
void inicializar_estructuras();
void pedir_memoria(int socket);
void recibir_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc);
void desempaquetar_parametros_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc);

extern t_log* logger_kernel;
extern int id_counter;
extern pthread_mutex_t m_hilo_en_ejecucion;
extern pthread_mutex_t m_hilo_a_ejecutar;
extern pthread_mutex_t m_proceso_en_ejecucion;
extern pthread_mutex_t m_proceso_a_ejecutar;
extern pthread_mutex_t m_lista_de_ready;
extern pthread_mutex_t m_regreso_de_cpu;
extern pthread_mutex_t m_lista_procesos_new;

extern sem_t finalizo_un_proc;

extern t_list* lista_de_ready;
extern t_list* lista_procesos_new;
void finalizar_tcb(tcb* hilo_a_finlizar);
extern char *ip_memoria;
extern char *puerto_memoria; 
extern char *ip_cpu; 
extern char *puerto_cpu_dispatch; 
extern char *puerto_cpu_interrupt; 
extern char *algoritmo_de_planificacion; 
extern int quantum; 
extern char *log_level; 
extern int conexion_dispatch;
extern int conexion_interrupt;
extern int conexion_memoria;
extern int conectarMemoria();