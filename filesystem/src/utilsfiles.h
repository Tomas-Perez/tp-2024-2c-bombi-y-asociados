#ifndef UTILSFILES_H_
#define UTILSFILES_H_

#include <utils/utils.h>

extern pthread_mutex_t mconteo;
extern uint32_t block_size,block_count,retardo_acceso_bloque;
extern char *puerto_escucha, *mount_dir, *log_level;
extern t_config *config_fs;


int redondeo_bloques(int tamanio);
void levantar_config_fs();

#endif