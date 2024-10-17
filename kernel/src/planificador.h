#include "utilsKernel.h"
extern int id_counter;

void pasar_a_running_tcb(tcb* tcb_listo);
void pasar_a_running_pcb(pcb* proceso_listo);
void pasar_a_running_tcb_prioridades();
void planificador_corto_plazo();
void agregar_a_ready(tcb* hilo);
void agregar_a_ready_multinivel(tcb* hilo);
tcb* hilo_prioritario_en_ready();
tcb* elegir_segun_prioridades();

