#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <utils/utils.h>

void levantar_config_fs();
void atenderMemoria();
void aceptar_peticiones(int socket_servidor);
void atender_petiticiones(int *socket);
void inicializarBloques();
void inicializarBitmap();
void archivocheq();

#endif