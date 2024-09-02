#include <utils/utils.h>

void saludar(char* quien) {
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

t_log* iniciar_logger(char* nombreLog, char* proceso){

	t_log* nuevo_logger= log_create(nombreLog,proceso,1,LOG_LEVEL_INFO);

	if(nuevo_logger ==NULL){
		 printf("No pude crear el logger\n");}
		 // si se debe meter en la funcion agrego exit(1);

	return nuevo_logger;
}

// -------------------------------- CONFIG --------------------------------

t_config* iniciar_config(char* archivo){

	t_config* nuevo_config= config_create(archivo);

	if(nuevo_config ==NULL){
		printf("No pude leer la config \n");
		exit(2);
	}

	return nuevo_config;
}
// -------------------------------- CONEXIONES: CLIENTE --------------------------------
void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}




// -------------------------------- CONEXIONES: SERVIDOR --------------------------------








// -------------------------------- ENVIO INFO --------------------------------



