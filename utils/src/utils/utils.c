#include <utils/utils.h>

void saludar(char *quien)
{
	printf("Hola desde %s!!\n", quien);
}
// -------------------------------- KERNEL --------------------------------
inicializar_estructuras_kernel()
{
	printf(" :) \n");
}

// -------------------------------- CPU --------------------------------

// -------------------------------- MEMORIA --------------------------------

// -------------------------------- FILESYSTEM --------------------------------

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
		log_info(logger, "Handshake exitoso entre cpu y memoria");
	}
	else
	{
		log_info(logger, "Handshake fallido entre cpu y memoria");
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
uint32_t buffer_read_uint32(t_buffer *buffer)
{
	uint32_t data;
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
uint8_t buffer_read_uint8(t_buffer *buffer)
{
	uint8_t data;
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

// Lee un string y su longitud del buffer y avanza el offset
char *buffer_read_string(t_buffer *buffer, uint32_t *length)
{
	*length = buffer_read_uint32(buffer); // Leer la longitud primero
	char *string = malloc(*length + 1);	  // +1 para el terminador NULL
	memcpy(string, buffer->stream + buffer->offset, *length);
	string[*length] = '\0'; // Terminar la cadena
	buffer->offset += *length;
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
void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
	// free(buffer);
}
