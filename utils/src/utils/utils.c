#include "utils.h"

// t_log* logger;

void saludar(char *quien)
{
	printf("Hola desde %s!!\n", quien);
}

// -------------------------------- LOGGER --------------------------------

t_log *iniciar_logger(char *nombreLog, char *proceso)
{

	t_log *nuevo_logger = log_create(nombreLog, proceso, 1, LOG_LEVEL_INFO);

	if (nuevo_logger == NULL)
	{
		printf("No pude crear el logger\n");
	}
	// si se debe meter en la funcion agrego exit(1);

	return nuevo_logger;
}

// -------------------------------- CONFIG --------------------------------

t_config *iniciar_config(char *archivo)
{

	t_config *nuevo_config = config_create(archivo);

	if (nuevo_config == NULL)
	{
		printf("No pude leer la config \n");
		exit(2);
	}

	return nuevo_config;
}
// -------------------------------- CONEXIONES: CLIENTE --------------------------------

int crear_conexion(char *ip, char *puerto)
{
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(ip, puerto, &hints, &server_info) != 0)
	{
		perror("Error en getaddrinfo");
		return -1;
	}

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (socket_cliente == -1)
	{
		perror("Error al crear socket");
		freeaddrinfo(server_info);
		return -1;
	}

	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		perror("Error en connect");
		close(socket_cliente);
		freeaddrinfo(server_info);
		return -1;
	}

	freeaddrinfo(server_info);
	return socket_cliente;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

void handshake_cliente(int socket_cliente, t_log *logger)
{
	size_t bytes;

	int32_t handshake = 1;
	int32_t result;
	uint32_t resultOk = 0;
	bytes = send(socket_cliente, &handshake, sizeof(int32_t), 0);
	bytes = recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);

	if (result == resultOk)
	{
		log_info(logger, "Handshake exitoso");
	}
	else
	{
		log_info(logger, "Handshake fallido");
	}
}

// -------------------------------- CONEXIONES: SERVIDOR --------------------------------

int iniciar_servidor(char *puerto)
{
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, puerto, &hints, &server_info) != 0)
	{
		perror("Error en getaddrinfo");
		return -1;
	}

	int socket_servidor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (socket_servidor == -1)
	{
		perror("Error al crear socket del servidor");
		freeaddrinfo(server_info);
		return -1;
	}

	if (bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		perror("Error en bind");
		close(socket_servidor);
		freeaddrinfo(server_info);
		return -1;
	}

	if (listen(socket_servidor, SOMAXCONN) == -1)
	{
		perror("Error en listen");
		close(socket_servidor);
		freeaddrinfo(server_info);
		return -1;
	}

	freeaddrinfo(server_info);
	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// aceptar cliente-hilo padre
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	if (socket_cliente == -1)
	{
		perror("Error al aceptar conexión del cliente");
		return -1;
	}
	// atiende el cliente-dif hilos
	uint32_t handshake;
	uint32_t resultOk = 0;
	uint32_t resultError = -1;
	size_t bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);

	if (bytes == -1)
	{
		perror("Error al recibir handshake del cliente");
		close(socket_cliente);
		return -1;
	}

	if (handshake == 1)
	{
		bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
		if (bytes == -1)
		{
			perror("Error al enviar handshake de respuesta al cliente");
			close(socket_cliente);
			return -1;
		}
	}
	else
	{
		bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);
		if (bytes == -1)
		{
			perror("Error al enviar handshake de error al cliente");
			close(socket_cliente);
			return -1;
		}
	}

	return socket_cliente;
}

// -------------------------------- ENVIO INFO --------------------------------

// Agrega un uint32_t al buffer
void buffer_add_uint32(t_buffer *buffer, uint32_t data)
{
	buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint32_t));
	memcpy(buffer->stream + buffer->size, &data, sizeof(uint32_t));
	buffer->size += sizeof(uint32_t);
}

// Lee un uint32_t del buffer y avanza el offset
uint32_t buffer_read_uint32(t_buffer *buffer) {
    uint32_t data = 0;

    if (buffer->offset + sizeof(uint32_t) > buffer->size) {
        //log_info(logger_memoria, "Error: Offset fuera de límites del buffer");
        return 0;  // o algún valor de error
    }

    memcpy(&data, buffer->stream + buffer->offset, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    return data;
}

// Agrega un uint8_t al buffer
void buffer_add_uint8(t_buffer *buffer, uint8_t data)
{
	buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
	memcpy(buffer->stream + buffer->size, &data, sizeof(uint8_t));
	buffer->size += sizeof(uint8_t);
}

// Lee un uint8_t del buffer y avanza el offset
uint8_t buffer_read_uint8(t_buffer *buffer) {
    uint8_t data = 0;

    if (buffer->offset + sizeof(uint8_t) > buffer->size) {
        //log_info(logger_memoria, "Error: Offset fuera de límites del buffer");
        return 0;  // o algún valor de error
    }

    memcpy(&data, buffer->stream + buffer->offset, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
    return data;
}

// Agrega string al buffer con un uint32_t indicando su longitud
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string)
{
	buffer_add_uint32(buffer, length); // Primero agregar la longitud
	buffer->stream = realloc(buffer->stream, buffer->size + length);
	memcpy(buffer->stream + buffer->size, string, length);
	buffer->size += length;
}

char *buffer_read_string(t_buffer *buffer) {
    // Verificar que el offset está dentro de los límites del buffer
    if (buffer->offset + sizeof(uint32_t) > buffer->size) {
        //log_info(logger_memoria, "Error: Offset fuera de límites al leer la longitud del string");
        //*length = 0;
        return NULL;
    }

    // Leer la longitud del string
    uint32_t length = buffer_read_uint32(buffer);

    // Verificar que el buffer tiene suficiente espacio para el string
    if (buffer->offset + length > buffer->size) {
        //log_info(logger_memoria, "Error: Tamaño insuficiente en el buffer para leer el string");
        length = 0;
        return NULL;
    }

    // Asignar memoria para el string (+1 para el terminador NULL)
    char *string = malloc(length + 1);
    if (string == NULL) {
        //log_info(logger_memoria, "Error al asignar memoria para el string");
        length = 0;
        return NULL;
    }

    // Copiar el string desde el buffer
    memcpy(string, buffer->stream + buffer->offset, length);
    string[length] = '\0'; // Añadir terminador NULL

    // Avanzar el offset después de leer el string
    buffer->offset += length;

    return string;
}


int recibir_operacion(int socket_cliente)
{
	int cod_op = 0;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
	{
		// printf("Código de operación recibido: %d\n", cod_op);
		return cod_op;
	}
	else
	{
		close(socket_cliente);
		return -1;
	}
}

t_buffer *recibir_buffer(int *tamaño_buffer, int socket_cliente) {
    void *buffer = NULL;

    // Intentar recibir el tamaño del buffer
    int bytes_recibidos = recv(socket_cliente, tamaño_buffer, sizeof(int), MSG_WAITALL);
    if (bytes_recibidos <= 0) {
        //log_info(logger_memoria, "Error al recibir el tamaño del buffer o conexión cerrada");
        return NULL;
    }

    // Asignar memoria para el buffer en función del tamaño recibido
    buffer = malloc(*tamaño_buffer);
    if (buffer == NULL) {
        //log_info(logger_memoria, "Error al asignar memoria para el buffer");
        return NULL;
    }

    // Intentar recibir el contenido del buffer
    bytes_recibidos = recv(socket_cliente, buffer, *tamaño_buffer, MSG_WAITALL);
    if (bytes_recibidos != *tamaño_buffer) {
        //log_info(logger_memoria, "Error al recibir el contenido del buffer o conexión cerrada");
        free(buffer);  // Liberar memoria antes de salir
        return NULL;
    }

	t_buffer *buffer_a_devolver = malloc(sizeof(t_buffer));
	buffer_a_devolver->offset = 0;
	buffer_a_devolver->size = *tamaño_buffer;
	buffer_a_devolver->stream = buffer;

    return buffer_a_devolver;
}

void* recibir_buffer_vieja(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
	//free(buffer);
}

void* recibir_buffer_mensaje(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
	//free(buffer);
}


void recibir_mensaje(int socket_cliente, t_log *logger)
{
	int size;
	char *buffer = recibir_buffer_mensaje(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

void enviar_mensaje(char *mensaje, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_buffer *crear_buffer(){
	t_buffer *buffer = malloc(sizeof(t_buffer));
	assert(buffer != NULL);

	buffer->size = 0;
	buffer->stream = NULL;
	return buffer;
}

t_paquete *crear_paquete(int codigo_operacion)
{
	t_paquete *paquete = (t_paquete *)malloc(sizeof(t_paquete));

	assert(paquete != NULL);

	paquete->codigo_operacion = codigo_operacion;
	paquete->buffer = crear_buffer();
	return paquete;
	// free(paquete);
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void agregar_a_paquete_solo(t_paquete *paquete, void *valor, int bytes)
{
	t_buffer *buffer = paquete->buffer;
	buffer->stream = realloc(buffer->stream, buffer->size + bytes);
	memcpy(buffer->stream + buffer->size, valor, bytes);
	buffer->size += bytes;
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
		//free(valor);
	}
	free(buffer);

	return valores;
}
//---------------------------------------------------------------------------------------------
const char* op_code_to_string(op_code code) {
    switch (code) {
        case MENSAJE: return "MENSAJE";
        case PAQUETE: return "PAQUETE";
        case SET: return "SET";
        case READ_MEM: return "READ_MEM";
        case WRITE_MEM: return "WRITE_MEM";
        case SUM: return "SUM";
        case SUB: return "SUB";
        case JNZ: return "JNZ";
        case LOG: return "LOG";
        case DUMP_MEMORY: return "DUMP_MEMORY";
        case IO: return "IO";
        case PROCESS_CREATE: return "PROCESS_CREATE";
        case THREAD_CREATE: return "THREAD_CREATE";
        case THREAD_JOIN: return "THREAD_JOIN";
        case THREAD_CANCEL: return "THREAD_CANCEL";
        case MUTEX_CREATE: return "MUTEX_CREATE";
        case MUTEX_LOCK: return "MUTEX_LOCK";
        case MUTEX_UNLOCK: return "MUTEX_UNLOCK";
        case THREAD_EXIT: return "THREAD_EXIT";
        case PROCESS_EXIT: return "PROCESS_EXIT";
        case NO_RECONOCIDO: return "NO_RECONOCIDO";
        case FETCH_INSTRUCCION: return "FETCH_INSTRUCCION";
        case SYSCALL: return "SYSCALL";
        case SUCCESS: return "SUCCESS";
        case INICIAR_HILO: return "INICIAR_HILO";
        case DESALOJAR_PROCESO: return "DESALOJAR_PROCESO";
        case OP_ENVIO_PCB: return "OP_ENVIO_PCB";
        case PEDIR_CONTEXTO: return "PEDIR_CONTEXTO";
        case ACTUALIZAR_CONTEXTO: return "ACTUALIZAR_CONTEXTO";
        case OP_ENVIO_TCB: return "OP_ENVIO_TCB";
        case RR: return "RR";
        case SEGMENTATION_FAULT: return "SEGMENTATION_FAULT";
        case VALOR_REGISTRO: return "VALOR_REGISTRO";
        default: return "UNKNOWN";
    }
}