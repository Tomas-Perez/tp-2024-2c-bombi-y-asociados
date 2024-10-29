#include <utils/utils.h>

void inicializar_hilo(t_proceso *proceso_padre, int tid, t_hilo *hilo_a_inicializar, FILE *f);
void inicializar_registros(t_hilo *hilo);
void guardar_instrucciones(t_hilo *hilo, FILE *f);
void empaquetar_contexto(t_paquete *paquete, t_proceso *proceso, t_hilo *hilo);
void actualizar_contexto_en_memoria(t_proceso *proceso, t_hilo *hilo, t_registros_cpu registros);
t_registros_cpu recibir_contexto(t_registros_cpu registros, void *buffer);

// FUNCIONES PARA INICIALIZAR MEMORIA

void inicializar_particiones_fijas();
void inicializar_particiones_dinamicas();
char* eliminar_corchetes(char* cad);
bool particion_mayor(void *a, void *b);

// FUNCIONES PARA MANEJO DE PARTICIONES

t_particiones *asignar_first_fit(t_list *lista, uint32_t tamanio);
t_particiones *asignar_best_fit(t_list *lista, uint32_t tamanio);
t_particiones *asignar_worst_fit(t_list *lista, uint32_t tamanio);




extern pthread_mutex_t mutex_instrucciones;
extern t_list *particiones_fijas;
extern t_list *particiones_dinamicas;
extern uint32_t tamanio_memoria;
extern char *particiones;


