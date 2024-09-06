#include <utils/utils.h>
char* generar_path_archivo(char* nombre_archivo);
void interpretar_archivo_pseudocodigo(char* path);
void abrir_e_interpretar_archivo_pseudocodigo(char* nombre_archivo);
pcb *crear_pcb();
tcb* crear_tid(pcb* pcb_padre, int prioridad);
void inicializar_registros(pcb* proc);

void inicializar_estructuras();

extern t_log* logger_kernel;
extern int id_counter;
extern pthread_mutex_t m_hilo_en_ejecucion;
extern pthread_mutex_t m_proceso_en_ejecucion;
extern pthread_mutex_t m_proceso_a_ejecutar;
extern pthread_mutex_t m_cola_de_ready;
extern pthread_mutex_t m_regreso_de_cpu;
extern t_queue* cola_de_ready;

