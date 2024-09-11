#include <utils/utils.h>
char* generar_path_archivo(char* nombre_archivo);
void interpretar_archivo_pseudocodigo(char* path);
void abrir_e_interpretar_archivo_pseudocodigo(char* nombre_archivo);
pcb *crear_pcb();
tcb* crear_tcb(pcb* proc_padre, int prioridad);
void inicializar_registros(pcb* proc);

void inicializar_estructuras();
void pedir_memoria(pcb* proceso_nuevo, int socket);
void recibir_syscall_de_cpu(pcb* proc, int* motivo, instruccion* instrucc);
void desempaquetar_parametros_syscall_de_cpu(pcb* proc, int* motivo, instruccion* instrucc);
extern t_log* logger_kernel;
extern int id_counter;
extern pthread_mutex_t m_hilo_en_ejecucion;
extern pthread_mutex_t m_proceso_en_ejecucion;
extern pthread_mutex_t m_proceso_a_ejecutar;
extern pthread_mutex_t m_lista_de_ready;
extern pthread_mutex_t m_regreso_de_cpu;
extern t_list* lista_de_ready;

extern char *ip_memoria;
extern char *puerto_memoria; 
extern char *ip_cpu; 
extern char *puerto_cpu_dispatch; 
extern char *puerto_cpu_interrupt; 
extern char *algoritmo_de_planificacion; 
extern int quantum; 
extern char *log_level; 
