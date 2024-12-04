#include "filesystem.h"

t_list* list_archivos;
int socket_cliente,fpbitmap,fpbloc;
char *ptr_bitarray;
uint32_t *blocmap;

sem_t semaforo,sem2;


int main(int argc, char *argv[])
{

    levantar_config_fs();
    logger_fs = iniciar_logger("filesystem.log", "FILESYSTEM");
    
    sem_init(&sem2,0,0);
    inicializarBloques();
    inicializarBitmap();
    archivocheq();
    bitmap_check();

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
    void* confirmemo;
    sem_init(&semaforo, 0, 0);
    while (1)
    {
        
        pthread_t thread;
        int socket_cliente = accept(socket_servidor, NULL, NULL);
        // puts("cliente aceptado");
        pthread_create(&thread, NULL, (void *)atender_petiticiones, &socket_cliente);
        sem_wait(&semaforo);
        pthread_join(thread, &confirmemo);
        send(socket_cliente,(int*)confirmemo,sizeof(int),0);
        free((int*)confirmemo);
        close(socket_cliente);
        // puts("siguiente hilo");
    }
}

void atender_petiticiones(int *socket)
{
    int socket_cliente = *socket;
    //printf("el socket memoria es %d \n",socket_cliente);
    //t_list *lista;
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
    //puts("handshake hecho");
    int koso = 0;
    int size =0;
    int* exit_status=malloc(sizeof(int));
    t_buffer *buffer;
    while (socket_cliente) // cambiar para registrar cada interfaz , importante guardar socket de cada interfaz
    {
        if (koso == 1)
        {
            break;
        }
        int cod_op = recibir_operacion(socket_cliente);
        switch (cod_op)
        {
        case DUMP_MEMORY:
            buffer=recibir_buffer(&size,socket_cliente);

            int pid=buffer_read_uint32(buffer);
            int tid=buffer_read_uint32(buffer);
            int tam=buffer_read_uint32(buffer);
            void* data=malloc(tam);
            recv(socket_cliente,data,tam,MSG_WAITALL);

            time_t timestamp= time(NULL);
            struct tm *tm = localtime(&timestamp);
            char* tiempo=malloc(sizeof(char)*50);
            strftime(tiempo,24,"%I:%M:%S%p",tm);
            char* nombr=string_from_format("%d-%d-%s",pid,tid,tiempo);
            if(crear_archivo(nombr,tam,socket_cliente,data)==-1){
                *exit_status=0;
                free(data);
                sem_post(&semaforo);
                pthread_exit(exit_status);
            }else{
                *exit_status=1;
                free(data);
                sem_post(&semaforo);
                pthread_exit(exit_status);
            }
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

void archivocheq(){
}

void bitmap_check(){
    sem_wait(&sem2);
    bits_disp= bitarray_get_max_bit(bitarray_bitmap);
    for(int i=0;i<block_count;i++)
    if(bitarray_test_bit(bitarray_bitmap,i)==true){
        bits_disp--;
    }
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
    blocmap = mmap(NULL, tamanio, PROT_READ|PROT_WRITE, MAP_SHARED, fpbloc, 0);
    if (blocmap == MAP_FAILED)
    {
        log_error(logger_fs, "Error al mapear bloques");
        exit(EXIT_FAILURE);
    }

    sem_post(&sem2);
}

void inicializarBitmap()
{
    char *archivo = string_from_format("%s/bitmap.dat", mount_dir);
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
    ptr_bitarray = mmap(NULL, tamanio, PROT_READ|PROT_WRITE, MAP_SHARED, fpbitmap, 0);
    if (ptr_bitarray == MAP_FAILED)
    {
        log_error(logger_fs, "Error al mapear bitarray.dat");
        close(fpbitmap);
        exit(EXIT_FAILURE);
    }

    //memset(ptr_bitarray, 0, tamanio);

    bitarray_bitmap = bitarray_create_with_mode(ptr_bitarray, tamanio, LSB_FIRST);
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
    tamanio_bloq_puntero=block_size/sizeof(uint32_t);   //bloques para guardar el contenido del mismo, ya que cada archivo tiene un único bloque de punteros.
    sem_post(&sem2);
}

int crear_archivo(char* nombre, int size,int socket_cli,void* data){
    int cant_bloques=redondeo_bloques(size);
    int bloque_disp=verificar_espacio_disp(bitarray_bitmap,cant_bloques);
    if (bloque_disp==-1){
        //mandar_error(socket_cli);
        log_error(logger_fs, "## Fin de solicitud - Archivo: %s No creado",nombre);
        return -1;
    }else{
    }
    log_info(logger_fs,"## Archivo Creado: %s- Tamaño: %d",nombre,size);
    reservar_bloques_bitmap(bitarray_bitmap,bloque_disp,cant_bloques,nombre); //paso 2
    crear_metadata(nombre,bloque_disp,size); //paso 3
    grabar_bloques(blocmap,bloque_disp,cant_bloques,nombre); //paso 4
    accerder_y_escribir_bloques(blocmap,bloque_disp,cant_bloques,data,size,nombre); //paso 5
    log_info(logger_fs, "## Fin de solicitud - Archivo: %s",nombre);
    return 0;
}
