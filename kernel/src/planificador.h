#include "utilsKernel.h"
extern int id_counter;

void pasar_a_running_tcb(tcb* tcb_listo);
void pasar_a_running_pcb(pcb* proceso_listo);
void planificador_corto_plazo_pcb();
void agregar_a_ready_prioridades(tcb* hilo);
void agregar_a_ready(tcb* hilo);