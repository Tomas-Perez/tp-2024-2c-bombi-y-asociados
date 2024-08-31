#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <time.h>
#include <commons/bitarray.h>
#include <math.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/string.h>
#include<assert.h>
#include<signal.h>
#include<string.h>
#include <pthread.h>
#include <commons/temporal.h>
#include <semaphore.h>

/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/
void saludar(char* quien);

#endif
