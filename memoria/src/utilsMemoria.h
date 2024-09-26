#include <utils/utils.h>

void inicializar_hilo(t_proceso *proceso_padre, int tid, t_hilo *hilo_a_inicializar, FILE *f);
void inicializar_registros(t_hilo *hilo);
void guardar_instrucciones(t_hilo *hilo, FILE *f);
void empaquetar_contexto(t_paquete *paquete, t_proceso *proceso, t_hilo *hilo);
void actualizar_contexto_en_memoria(t_proceso *proceso, t_hilo *hilo, t_registros_cpu registros);
t_registros_cpu recibir_contexto(t_registros_cpu registros, void *buffer);

extern pthread_mutex_t mutex_instrucciones;


