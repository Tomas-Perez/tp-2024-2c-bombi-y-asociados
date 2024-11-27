#include <utils/utils.h>
char* generar_path_archivo(char* nombre_archivo);
pcb *crear_pcb(int prioridad, char* path, int tamanio, int socket);
tcb* crear_tcb(pcb* proc_padre, int prioridad);
void inicializar_registros(tcb* hilos);
void finalizar_hilos_proceso(pcb* proceso);
pcb *buscar_proc_lista(t_list *lista, int pid_buscado);
tcb* buscar_TID(tcb* tcb_pedido, int tid_buscado);
tcb* buscar_hilos_listas(tcb* main, int tid);
tcb* buscar_hilo_en_multinivel(int prioridad, int tid);
void inicializar_estructuras();
void iniciar_hilo(tcb* hilo, int conexion_memoria, char* path);
void pedir_memoria(int socket);
void desalojar_hilo(int motivo);
void* desalojar_por_RR(tcb* hilo);
void recibir_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc);
void desempaquetar_parametros_syscall_de_cpu(tcb* hilo, int* motivo, instruccion* instrucc);
int bloquear_por_dump(tcb* hilo, int socket);
void finalizar_proceso(pcb *proc);
void liberar_bloqueados_x_thread_join(tcb* hilo) ;
void liberar_param_instruccion(instruccion* instrucc);
extern void agregar_a_ready_multinivel(tcb* hilo);
extern void agregar_a_ready(tcb* hilo); // TO DO: preguntar si esto es legal
void finalizar_estructuras_kernel();
void inicializar_estructuras_kernel();
void finalizar_tcb(tcb* hilo_a_finlizar);
void sacar_de_lista_pcb(tcb* hilo_a_sacar);
void liberar_mutexs_asociados(tcb* hilo);
void asignar_mutex_al_primer_bloqueado(mutex_k* mutex_solic);
void inicializar_hilos_planificacion();
bool existe_mutex(mutex_k* mutex_solic,t_list* lista_mutex_proceso);
void asignar_mutex_hilo(mutex_k* mutex,tcb* hilo);
bool mutex_tomado_por_hilo(mutex_k* mutex, tcb* hilo);
nivel_prioridad* encontrar_por_nivel(t_list* lista_multinivel, int prioridad);
nivel_prioridad* encontrar_nivel_mas_prioritario(t_list* multinivel);
void agregar_a_ready_segun_alg(tcb* hilo);
mutex_k* crear_mutex(char* nombre);
void crear_cola_nivel(int prioridad,tcb* hilo, nivel_prioridad* nuevo_nivel);
void* hilo_exit();
void avisar_memoria_liberar_pcb(pcb* proc);
void avisar_memoria_liberar_tcb(tcb* hilo);
void liberar_tcb(tcb* hilo);
void mandar_tcb_dispatch(tcb*);
int verificar_lista_ready(t_list* lista_de_ready);
tcb* buscar_tid(t_list* lista_tcb,int tid);
extern t_log* logger_kernel;
extern int id_counter;
extern syscall_solicitada;
extern tcb* hilo_en_ejecucion; 
extern pthread_mutex_t m_hilo_en_ejecucion;
extern pthread_mutex_t m_hilo_a_ejecutar;
extern pthread_mutex_t m_proceso_en_ejecucion;
extern pthread_mutex_t m_proceso_a_ejecutar;
extern pthread_mutex_t m_lista_de_ready;
extern pthread_mutex_t m_regreso_de_cpu;
extern pthread_mutex_t m_lista_procesos_new;
extern pthread_mutex_t m_lista_de_bloqueados;
extern pthread_mutex_t m_lista_multinivel;
extern pthread_mutex_t m_lista_finalizados;
extern pthread_mutex_t m_lista_prioridad;
extern pthread_mutex_t m_syscall_solicitada;
extern pthread_mutex_t m_lista_io;
extern sem_t finalizo_un_proc;
extern sem_t hilos_en_exit;
extern sem_t hilos_en_ready;
extern sem_t binario_corto_plazo;

extern t_list* lista_de_ready;
extern t_list* lista_procesos_new;
extern t_list* lista_multinivel;
extern t_list* lista_finalizados;
extern t_list* lista_io;
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
extern t_config* config_kernel;
extern int conectarMemoria();
extern void planificador_corto_plazo();
void levantar_config_kernel();
