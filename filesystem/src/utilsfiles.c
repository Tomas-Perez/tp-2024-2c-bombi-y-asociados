#include "utilsfiles.h"
t_bitarray *bitarray_bitmap;
pthread_mutex_t mconteo;
int block_size,block_count,retardo_acceso_bloque,bits_disp;
char *puerto_escucha, *mount_dir, *log_level;
t_config *config_fs;
int tamanio_bloq_puntero;
t_log *logger_fs;

void levantar_config_fs()
{
    //config_fs = config_create("configs/filesystemPlani.config");
    config_fs = config_create("configs/filesystemFS.config");
    //config_fs = config_create("configs/filesystemRC.config");
    //config_fs = config_create("configs/filesystemParticionesFijas.config");
    //config_fs = config_create("configs/filesystemParticionesDinamicas.config");
   

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
    char *direccion_metadata = string_from_format("%s/files/%s.dmp", config_get_string_value(config_fs, "MOUNT_DIR"), nombre);
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

int verificar_espacio_disp(t_bitarray* bit,int size){
    //int bits_disp=(block_count/8)-bits_usados;
    if (bits_disp<size){
        return -1;
    }else{
    int busqueda;
    for (int i = 0; i < block_count-1; i++)
    {
        bool test = bitarray_test_bit(bit, i);
        if (test == false)
        {
            busqueda = i;
            break;
        }
    }
    bits_disp--;
    return busqueda;
    }
}

void reservar_bloques_bitmap(t_bitarray* bit,int bloque_disp,int cant_bloques,char* nombre){
    bitarray_set_bit(bit,bloque_disp);
    log_info(logger_fs,"## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d",bloque_disp,nombre,bits_disp);
    for(int i=1;i<cant_bloques+1;i++){
        bitarray_set_bit(bit,bloque_disp+i);
        bits_disp--;
        log_info(logger_fs,"## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d",bloque_disp+i,nombre,bits_disp);
    }
}

void mandar_error(int socke){
    enviar_mensaje("no se pudo crear archivo de la operacion",socke);
}
void grabar_bloques(uint32_t* blocmap,int bloque_disp,int cant_bloques,char* nombre){
    int bloque_index=bloque_disp*tamanio_bloq_puntero;
    int* source=malloc(sizeof(int));
    int j=1;
    for(int i=0;i<cant_bloques;i++){
        *source=bloque_disp*block_size+block_size*j;
        memcpy(blocmap+bloque_index+i,source,sizeof(int));
        j++;
    }
    log_info(logger_fs,"## Acceso Bloque - Archivo: %s - Tipo Bloque: ÍNDICE - Bloque File System %d",nombre,bloque_disp*block_size);
    usleep(1000*retardo_acceso_bloque);
    free(source);
}
void accerder_y_escribir_bloques(uint32_t* blocmap,int bloque_disp,int cant_bloques,void* data,int size,char* nombre){
    int bloque_index_s=bloque_disp*block_size;
    int j=1;
    if(size<=block_size){
        memcpy((void*)blocmap+(bloque_index_s+block_size),data,size);
        log_info(logger_fs,"## Acceso Bloque - Archivo: %s - Tipo Bloque: DATOS - Bloque File System %d",nombre,(bloque_index_s+block_size));
        usleep(1000*retardo_acceso_bloque);
    }else{
        int aux=size;
        int size_aux[cant_bloques];
        int h=0;
        while(aux>block_size){
            size_aux[h]=block_size;
            aux=aux-block_size;
            h++;
        }
        size_aux[h]=aux;
        for(int i=0;i<cant_bloques;i++){
            memcpy((void*)blocmap+(bloque_index_s+(block_size*j)),data+(block_size*i),size_aux[i]);
            log_info(logger_fs,"## Acceso Bloque - Archivo: %s - Tipo Bloque: DATOS - Bloque File System %d",nombre,(bloque_index_s+(block_size)*j));
            usleep(1000*retardo_acceso_bloque);
            j++;
        }
    }
}
