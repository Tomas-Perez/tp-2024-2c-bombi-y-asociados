#include "filesystem.h"

t_config *config_fs;
t_log *logger_fs;
t_list* list_archivos;
char *puerto_escucha, *mount_dir, *log_level;
uint32_t block_size,block_count,retardo_acceso_bloque;
int socket_cliente,fpbitmap,fpbloc;
t_bitarray *bitarray_bitmap;
char *ptr_bitarray;
void *blocmap;

sem_t semaforo,sem2;

typedef struct
{
    char* nombre;
    int bloque_inicial;
    int tamanio;
    int cant_bloque;
}t_archivo;

int main(int argc, char *argv[])
{

    levantar_config_fs();
    logger_fs = iniciar_logger("filesystem.log", "FILESYSTEM");
    
    sem_init(&sem2,0,0);
    inicializarBloques();
    inicializarBitmap();
    archivocheq();
    // HILOS PARA CONEXIONES

    pthread_t t1;
    pthread_create(&t1, NULL, (void *)atenderMemoria, NULL);
    pthread_join(t1, NULL);

    return 0;
}
void atenderMemoria()
{
    char *puerto = puerto_escucha;
    int server_fs = iniciar_servidor(puerto); // socket, bind y listen
    log_info(logger_fs, "Servidor listo para recibir al cliente");
    aceptar_peticiones(server_fs);
}

void aceptar_peticiones(int socket_servidor)
{
    void* valrec;
    while (1)
    {
        sem_init(&semaforo, 0, 0);
        pthread_t thread;
        int socket_cliente = accept(socket_servidor, NULL, NULL);
        // puts("cliente aceptado");
        pthread_create(&thread, NULL, (void *)atender_petiticiones, &socket_cliente);
        sem_wait(&semaforo);
        pthread_join(thread, &valrec);
        enviar_mensaje((char*)valrec,socket_cliente);
        close(socket_cliente);
        // puts("siguiente hilo");
    }
}

void atender_petiticiones(int *socket)
{
    int socket_cliente = *socket;
    t_list *lista;
    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;
    // interfaz_recibida* nueva_interfaz;
    size_t bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);

    if (bytes == -1)
    {
        perror("Error al recibir handshake del cliente");
        close(socket_cliente);
    }

    if (handshake == 1)
    {
        bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
        if (bytes == -1)
        {
            perror("Error al enviar handshake de respuesta al cliente");
            close(socket_cliente);
        }
    }
    else
    {
        bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);
        if (bytes == -1)
        {
            perror("Error al enviar handshake de error al cliente");
            close(socket_cliente);
        }
    }
    int koso = 0;
    while (socket_cliente) // cambiar para registrar cada interfaz , importante guardar socket de cada interfaz
    {
        if (koso == 1)
        {
            break;
        }
        int cod_op = recibir_operacion(socket_cliente);
        switch (cod_op)
        {
        case MENSAJE:
            recibir_mensaje(socket_cliente,logger_fs);
            pthread_exit("Peticion exitosa");
            break;
        case PAQUETE:
            //lista = recibir_paquete(socket_cliente);
            puts("Me llegaron los siguientes valores:\n");
            sem_post(&semaforo);
            koso = 1;
            break;
        case -1:
            perror(string_from_format("el cliente de socket %d se desconecto", socket_cliente));
            close(socket_cliente);
            break;
        default:
            perror("WARNING: Operacion desconocida. No quieras meter la pata");
            break;
        }
    }
}

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

void archivocheq(){
    sem_wait(&sem2);
     t_config* aux;
     char* directorio=string_from_format("%s/files",mount_dir);
    DIR *d;
  struct dirent *dir;
  d = opendir(directorio);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      //printf("%s\n", dir->d_name);
      char* auxiliar=malloc(sizeof(char)*20);
      strcpy(auxiliar,dir->d_name);
      if (strstr(auxiliar, "txt.metada") != NULL) {
        aux=iniciar_config(string_from_format("%s/%s",directorio,auxiliar));
        t_archivo *nuevo=malloc(sizeof(t_archivo));
        char* archivo=config_get_string_value(aux, "NOMBRE");
        nuevo->nombre=malloc(strlen(archivo)+1);
        strcpy(nuevo->nombre,archivo);
        nuevo->bloque_inicial=config_get_int_value(aux,"BLOQUE_INICIAL");
        nuevo->tamanio=config_get_int_value(aux,"TAMAÑO_ARCHIVO");
        nuevo->cant_bloque=CANT_BLOQUES(nuevo->tamanio);
        list_add(list_archivos,nuevo);
        for (int i=nuevo->bloque_inicial;i<nuevo->bloque_inicial+nuevo->cant_bloque;i++){
        bitarray_set_bit(bitarray_bitmap,i);
        }
        int tamanioBitmap = block_count / 8;
        if (msync(ptr_bitarray, tamanioBitmap, MS_SYNC) == -1)
        {
        log_error(logger_fs, "Error al sincronizar el archivo bitmap.");
        }
    }
    free(auxiliar);
    }
    }
    if (aux->path!=NULL){
    config_destroy(aux);
    }
    closedir(d);
}

void inicializarBloques()
{
    int tamanio = block_count * block_size;
    char *archivo = string_from_format("%s/bloques.dat", mount_dir);
    fpbloc = open(archivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fpbloc == -1)
    {
        log_error(logger_fs, "no se pudo crear bloques.dat");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fpbloc, tamanio) == -1)
    {
        log_error(logger_fs, "no se pudo asignar tamaño a bloques");
        exit(EXIT_FAILURE);
    }
    blocmap = mmap(NULL, tamanio, PROT_WRITE, MAP_SHARED, fpbloc, 0);
    if (blocmap == MAP_FAILED)
    {
        log_error(logger_fs, "Error al mapear bloques");
        exit(EXIT_FAILURE);
    }
}

void inicializarBitmap()
{
    char *archivo = string_from_format("%s/bitmap.dat", mount_dir);
    //int tamanio = interfaz->block_count / 8;
    int tamanio= BIT_CHAR(block_count);
    fpbitmap = open(archivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fpbitmap == -1)
    {
        log_error(logger_fs, "no se pudo crear bitarray");
        close(fpbitmap);
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fpbitmap, tamanio) == -1)
    {
        log_error(logger_fs, "no se pudo asignar tamaño a bitarray");
        close(fpbitmap);
        exit(EXIT_FAILURE);
    }
    ptr_bitarray = mmap(0, tamanio, PROT_READ|PROT_WRITE, MAP_SHARED, fpbitmap, 0);
    if (ptr_bitarray == MAP_FAILED)
    {
        log_error(logger_fs, "Error al mapear bitarray.dat");
        close(fpbitmap);
        exit(EXIT_FAILURE);
    }

    memset(ptr_bitarray, 0, tamanio);

    bitarray_bitmap = bitarray_create_with_mode(ptr_bitarray, block_count, LSB_FIRST);
    if (!bitarray_bitmap)
    {
        log_error(logger_fs, "Error al asignar memoria al bitarray");
        munmap(ptr_bitarray, tamanio);
        close(fpbitmap);
        exit(EXIT_FAILURE);
    }
    if (msync(ptr_bitarray, bitarray_bitmap->size, MS_SYNC) == -1)
    {
        printf("Error en la sincronización con msync()\n");
        munmap(ptr_bitarray, tamanio);
        close(fpbitmap);
        exit(EXIT_FAILURE);
    }
    sem_post(&sem2);
}