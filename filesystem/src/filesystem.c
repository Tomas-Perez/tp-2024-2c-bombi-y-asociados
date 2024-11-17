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
    

    int pid=1;
    int tid=2;
    time_t timestamp= time(NULL);
    struct tm *tm = localtime(&timestamp);
    char* tiempo=malloc(sizeof(char)*50);
    strftime(tiempo,24,"%I:%M:%S%p",tm);
    char* nombr=string_from_format("%d-%d-%s",pid,tid,tiempo);

    int tamani=10;
    //void* data="I'm so glad you made time to see meHow's life? Tell me how's your familyI haven't seen them in a whileYou've been good, busier than everWe small talk, work and the weatherYour guard is up and I know whyBecause the last time you saw meIs still burning in the back of your mindYou gave me roses and I left them there to die";
    void* data="123456789";
    socket_cliente=1;
    crear_archivo(nombr,tamani,socket_cliente,data); //CODIGO DE PRUEBA PARA AL RECIBIR DATOS

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
        case DUMP_MEMORY:
            lista = recibir_paquete(socket_cliente);
            int* pid=list_get(lista,0);
            int* tid=list_get(lista,1);
            int* tam=list_get(lista,2);
            int tamani=*tam;
            void* data=list_get(lista,3);
            time_t timestamp= time(NULL);
            struct tm *tm = localtime(&timestamp);
            char* tiempo=malloc(sizeof(char)*50);
            strftime(tiempo,24,"%I:%M:%S%p",tm);
            char* nombr=string_from_format("%d-%d-%s",*pid,*tid,tiempo);
            if(crear_archivo(nombr,tamani,socket_cliente,data)==-1){
                pthread_exit(string_from_format("peticion %s fallida",nombr));
            }else{
                pthread_exit(string_from_format("peticion %s exitosa",nombr));
            }
            free(nombr);
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

void archivocheq(){
//    sem_wait(&sem2);
//     t_config* aux;
//     char* directorio=string_from_format("%s/files",mount_dir);
//    DIR *d;
//  struct dirent *dir;
//  d = opendir(directorio);
//  if (d) {
//    while ((dir = readdir(d)) != NULL) {
//      //printf("%s\n", dir->d_name);
//      char* auxiliar=malloc(sizeof(char)*20);
//      strcpy(auxiliar,dir->d_name);
//      if (strstr(auxiliar, ".dmp") != NULL) {
//        aux=iniciar_config(string_from_format("%s/%s",directorio,auxiliar));
//        t_archivo *nuevo=malloc(sizeof(t_archivo));
//        char* archivo=config_get_string_value(aux, "NOMBRE");
//        nuevo->nombre=malloc(strlen(archivo)+1);
//        strcpy(nuevo->nombre,archivo);
//        nuevo->index_block=config_get_int_value(aux,"INDEX_BLOCK");
//        nuevo->block_size=config_get_int_value(aux,"SIZE");
//        nuevo->cant_bloque=redondeo_bloques(nuevo->block_size);
//        list_add(list_archivos,nuevo);
//        for (int i=nuevo->index_block;i<nuevo->index_block+nuevo->cant_bloque;i++){     //ESTA FUNCION SOLO SIRVE PARA BLOQUES CONTINUOS, HAY QUE CAMBIARLO PARA QUE FUNCIONE EN INDEXADO
//        bitarray_set_bit(bitarray_bitmap,i);
//        }
//        int tamanioBitmap = block_count / 8;
//        if (msync(ptr_bitarray, tamanioBitmap, MS_SYNC) == -1)
//        {
//        log_error(logger_fs, "Error al sincronizar el archivo bitmap.");
//        }
//    }
//    free(auxiliar);
//    }
//    }
//    if (aux->path!=NULL){
//    config_destroy(aux);
//    }
//    closedir(d);
}

void bitmap_check(){
    sem_wait(&sem2);
    bits_disp= bitarray_get_max_bit(bitarray_bitmap);
    for(int i=0;i<BIT_CHAR(block_count);i++)
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

    //memset(blocmap, 0, tamanio);                                //retorna todo bloques.dat en 0
    //----------------------------------------------------------CODIGO DE PRUEBA DE ESCRITURA DE BLOQUES
    //char A='a';                                       
    //for (int i=(16*3);i<block_size;i++ ){//16 bytes 
    //    blocmap[i]=A;
    //}
    //int bloque_disp=0;
    //int cant_bits=3;
    //int j=1;
    //for(int i=bloque_disp*tamanio_bloq_puntero;i<cant_bits;i++){           
    //    blocmap[i]=bloque_disp+j;
    //    j++;
    //}
    //blocmap[tamanio_bloq_puntero]=1;
    //blocmap[tamanio_bloq_puntero+1]=2;

    //int* source=malloc(sizeof(int));
    //*source=1;
    //int* source2=malloc(sizeof(int));
    //*source2=2;
    //memcpy(blocmap,source,sizeof(int));
    //memcpy((void*)blocmap+14,source,10);
    //memcpy(blocmap+1,source2,sizeof(int));
    //free(source);
    //free(source2);
    //----------------------------------------------------------
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
    tamanio_bloq_puntero=block_size/sizeof(uint32_t);
    sem_post(&sem2);
}

int crear_archivo(char* nombre, int size,int socket_cli,void* data){
    //int index_block;
    int cant_bloques=redondeo_bloques(size);
    //block_size / sizeof(uint32_t); bloques para guardar el contenido del mismo, ya que cada archivo tiene un único bloque de punteros.
    int bloque_disp=verificar_espacio_disp(bitarray_bitmap,cant_bloques);
    if (bloque_disp==-1){
        mandar_error(socket_cli);
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
