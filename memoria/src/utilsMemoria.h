#include <utils/utils.h>

void inicializar_hilo(t_proceso *proceso_padre, int tid, t_hilo *hilo_a_inicializar, FILE *f);
void inicializar_registros(t_hilo *hilo, t_proceso *proceso);
void guardar_instrucciones(t_hilo *hilo, FILE *f);
void empaquetar_contexto(t_paquete *paquete, t_proceso *proceso, t_hilo *hilo);
void actualizar_contexto_en_memoria(t_proceso *proceso, t_hilo *hilo, t_registros_cpu registros);
t_registros_cpu recibir_contexto(t_registros_cpu registros, t_buffer *buffer);

// FUNCIONES PARA INICIALIZAR MEMORIA

void inicializar_particiones_fijas();
void inicializar_particiones_dinamicas();
char* eliminar_corchetes(char* cad);
bool particion_mayor(void *a, void *b);
bool base_menor(void *a, void *b);
void liberar_espacio_memoria(t_proceso *proceso);

// FUNCIONES PARA MANEJO DE PARTICIONES

t_particiones *asignar_first_fit_fijas(t_list *lista, uint32_t tamanio);
t_particiones *asignar_best_fit_fijas(t_list *lista, uint32_t tamanio);
t_particiones *asignar_worst_fit_fijas(t_list *lista, uint32_t tamanio);


t_particiones *asignar_first_fit_dinamicas(t_list *lista, uint32_t tamanio);
t_particiones *asignar_best_fit_dinamicas(t_list *lista, uint32_t tamanio);
t_particiones *asignar_worst_fit_dinamicas(t_list *lista, uint32_t tamanio);


t_particiones *buscar_particion(t_list *lista, uint32_t base, uint32_t limite);

void log_particiones(t_list* lista);

//FUNCIONES PARA FINALIZAR MEMORIA
void verificar_particiones_vecinas(t_list *lista);
void liberar_espacio_memoria(t_proceso *proceso);
void ordenar_lista_original(t_list *lista);


extern pthread_mutex_t mutex_instrucciones;
extern pthread_mutex_t m_lista_particiones;
extern t_list *lista_particiones;
extern uint32_t tamanio_memoria;
extern char *particiones;
extern t_log *logger_memoria;
extern char *esquema;
