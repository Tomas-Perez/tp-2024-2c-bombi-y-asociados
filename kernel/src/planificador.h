#include "utilsKernel.h"
extern int id_counter;
extern int indice; //permitime dudar 

void pasar_a_running_tcb(tcb* tcb_listo);
void pasar_a_running_pcb(pcb* proceso_listo);
void pasar_a_running_tcb_prioridades();
void planificador_corto_plazo_pcb();
void agregar_a_ready_prioridades(tcb* hilo); //este con lo de ahora no iria creo¿
void agregar_a_ready(tcb* hilo);
tcb* hilo_prioritario_en_ready();
tcb* elegir_segun_prioridades();
