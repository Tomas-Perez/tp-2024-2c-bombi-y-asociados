#ifndef UTILSFILES_H_
#define UTILSFILES_H_

#include <utils/utils.h>

extern pthread_mutex_t mconteo;
extern int block_size,block_count,retardo_acceso_bloque;
extern char *puerto_escucha, *mount_dir, *log_level;
extern t_config *config_fs;
extern int tamanio_bloq_puntero;
extern t_log *logger_fs;
extern t_bitarray *bitarray_bitmap;
extern int bits_disp;

int redondeo_bloques(int tamanio);
void levantar_config_fs();
void crear_metadata(char* nombre,int index_block,int size);
int verificar_espacio_disp(t_bitarray* bit,int size);
void reservar_bloques_bitmap(t_bitarray* bit,int bloque_disp,int cant_bloques,char*);
void mandar_error(int);
void grabar_bloques(uint32_t* blocmap,int bloque_disp,int cant_bloques,char*);
void accerder_y_escribir_bloques(uint32_t* blocmap,int bloque_disp,int cant_bloques,void*,int,char*);
#endif