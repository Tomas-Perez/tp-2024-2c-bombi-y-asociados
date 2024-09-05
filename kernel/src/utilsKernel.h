#include <utils/utils.h>
char* generar_path_archivo(char* nombre_archivo);
void interpretar_archivo_pseudocodigo(char* path);
void abrir_e_interpretar_archivo_pseudocodigo(char* nombre_archivo);
pcb *crear_pcb();
tcb* crear_tid(pcb* pcb_padre, int prioridad);
void inicializar_registros(pcb* proc);

void inicializar_estructuras();
extern int id_counter;