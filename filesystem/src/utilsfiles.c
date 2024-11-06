#include "utilsfiles.h"
pthread_mutex_t mconteo;
uint32_t block_size,block_count,retardo_acceso_bloque;
char *puerto_escucha, *mount_dir, *log_level;
t_config *config_fs;

void levantar_config_fs()
{
    config_fs = config_create("filesystem.config");
    puerto_escucha = config_get_string_value(config_fs, "PUERTO_ESCUCHA");
    mount_dir = config_get_string_value(config_fs, "MOUNT_DIR");
    log_level = config_get_string_value(config_fs, "LOG_LEVEL");
    block_size = config_get_int_value(config_fs, "BLOCK_SIZE");
    block_count = config_get_int_value(config_fs, "BLOCK_COUNT");
    retardo_acceso_bloque = config_get_int_value(config_fs, "RETARDO_ACCESO_BLOQUE");
}

int redondeo_bloques(int tamanio){
    int bloques;
    if (tamanio%block_size==0){
        bloques= tamanio/block_size;
        return bloques;
    }else if(tamanio%block_size!=0){
        while(tamanio%block_size!=0){
            pthread_mutex_lock(&mconteo);
            tamanio++;
            pthread_mutex_unlock(&mconteo);
        }  
        
    }
    bloques= tamanio/block_size;
    return bloques;
}

void crear_metadata(char* nombre,int index_block,int size){
    char *direccion_metadata = string_from_format("%s/%s.dmp", config_get_string_value(config_fs, "PATH_BASE_DIALFS"), nombre);
        FILE* meta=fopen(direccion_metadata, "w");
        t_config *metadata = iniciar_config(direccion_metadata);
        char* buffersize= string_from_format("%d",size);
        char* bufferindex= string_from_format("%d",index_block);
        config_set_value(metadata, "SIZE", buffersize);
        config_set_value(metadata, "INDEX_BLOCK", bufferindex);
        config_save(metadata);
        config_destroy(metadata);
        fclose(meta);
}