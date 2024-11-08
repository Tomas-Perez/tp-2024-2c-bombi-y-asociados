#ifndef UTILSFILES_H_
#define UTILSFILES_H_

#include <utils/utils.h>

extern pthread_mutex_t mconteo;
extern uint32_t block_size,block_count,retardo_acceso_bloque;
extern char *puerto_escucha, *mount_dir, *log_level;
extern t_config *config_fs;


int redondeo_bloques(int tamanio);
void levantar_config_fs();
void crear_metadata(char* nombre,int index_block,int size);
void reservar_bloques_bitmap();
int verificar_espacio_disp(t_bitarray* bit,int size);
void reservar_bloques_bitmap(t_bitarray* bit,int bloque_disp,int cant_bloques);
void mandar_error(int);
void grabar_bloques(char* blocmap,int bloque_disp,int cant_bloques);
void accerder_y_escribir_bloques(char* blocmap,int bloque_disp,int cant_bloques);
#endif