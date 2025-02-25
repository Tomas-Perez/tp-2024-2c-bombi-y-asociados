#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "utilsfiles.h"

typedef struct
{
    char* nombre;
    int index_block;
    int block_size;
    int cant_bloque;
}t_archivo;

void atenderMemoria();
void aceptar_peticiones(int socket_servidor);
void atender_petiticiones(int *socket);
void inicializarBloques();
void inicializarBitmap();
void archivocheq();
void bitmap_check();
int crear_archivo(char* nombre, int size,int,void*);

#endif