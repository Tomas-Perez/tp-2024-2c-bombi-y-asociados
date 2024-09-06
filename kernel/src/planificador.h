#include "utilsKernel.h"
extern int id_counter;

void pasar_a_running_tcb(tcb* tcb_listo);
void pasar_a_running_pcb(pcb* proceso_listo);
void planificador_corto_plazo_pcb();