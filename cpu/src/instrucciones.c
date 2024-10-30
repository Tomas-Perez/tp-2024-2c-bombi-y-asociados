#include "instrucciones.h"

bool ejecutando_un_proceso;
t_registros_cpu registros_cpu;
int tid;
int pid;

// Ciclo de instrucciones

void ejecutar_proceso()
{
    while (ejecutando_un_proceso)
    {
        check_interrupt(execute(decode(fetch())));
    }
}

void check_interrupt(instruccion *inst)
{
    if (interrupcion && ejecutando_un_proceso)
    {
        devolver_contexto_de_ejecucion(pid, tid);
        ejecutando_un_proceso = false;
    } // hay interrupcion y un proceso en ejecucion
    interrupcion = false;
    for (int i = 0; i < list_size(inst->parametros); i++)
    {
        char *parametro = list_remove(inst->parametros, i);

        // free(parametro);
    } // liberar cada parametro de instruccion
    list_destroy(inst->parametros);
    free(inst);
}

char *fetch()
{
    log_info(logger_cpu, "TID: <%d> - FETCH - Program Counter: <%d>", tid, registros_cpu.PC);
    t_paquete *paquete = crear_paquete(FETCH_INSTRUCCION);
    agregar_a_paquete_solo(paquete, &pid, sizeof(int));
    agregar_a_paquete_solo(paquete, &tid, sizeof(int));
    agregar_a_paquete_solo(paquete, &registros_cpu.PC, sizeof(uint32_t));

    enviar_paquete(paquete, socket_memoria);

    char *instruccion_a_ejecutar = recibir_instruccion(socket_memoria);
    // program_counter++;
    registros_cpu.PC++;
    log_info(logger_cpu, "Program Counter actualizado: %d", registros_cpu.PC);
    eliminar_paquete(paquete);
    return instruccion_a_ejecutar;
}

instruccion *decode(char *inst)
{
    instruccion *instruccion_decodificada = malloc(sizeof(instruccion));

    char **vector_terminos = string_split(inst, " ");

    u_int8_t identificador = get_identificador(vector_terminos[0]);

    // printf("IDENTIFICADOR INSTRUCCION: %i\n", identificador);

    for (int i = 0; i < string_array_size(vector_terminos); i++)
    {
        free(vector_terminos[i]);
    }

    free(vector_terminos);

    instruccion_decodificada->identificador = identificador;
    instruccion_decodificada->cant_parametros = get_cant_parametros(identificador); // ver esta funcion
    instruccion_decodificada->parametros = get_parametros(inst, instruccion_decodificada->cant_parametros);
    free(inst);
    return instruccion_decodificada;
}

instruccion *execute(instruccion *inst)
{
    switch (inst->identificador)
    {
    case SET:
        mostrar_parametros(inst, inst->cant_parametros);
        set(inst);
        break;
    case SUM:
        mostrar_parametros(inst, inst->cant_parametros);
        sum(inst);
        break;
    case SUB:
        mostrar_parametros(inst, inst->cant_parametros);
        sub(inst);
        break;
    case JNZ:
        mostrar_parametros(inst, inst->cant_parametros);
        jnz(inst);
        break;
    case READ_MEM:
        mostrar_parametros(inst, inst->cant_parametros);
        read_mem(inst);
        break;
    case WRITE_MEM:
        mostrar_parametros(inst, inst->cant_parametros);
        write_mem(inst);
        break;
    case LOG:
        mostrar_parametros(inst, inst->cant_parametros);
        log_instruccion(inst);
        break;
    case DUMP_MEMORY:
        mostrar_parametros(inst, inst->cant_parametros);
        dump_memory(inst);
        break;
    case IO:
        mostrar_parametros(inst, inst->cant_parametros);
        io(inst);
        break;
    case PROCESS_CREATE:
        mostrar_parametros(inst, inst->cant_parametros);
        process_create(inst);
        break;
    case THREAD_CREATE:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_create(inst);
        break;
    case THREAD_JOIN:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_join(inst);
        break;
    case THREAD_CANCEL:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_cancel(inst);
        break;
    case MUTEX_CREATE:
        mostrar_parametros(inst, inst->cant_parametros);
        mutex_create(inst);
        break;
    case MUTEX_LOCK:
        mostrar_parametros(inst, inst->cant_parametros);
        mutex_lock(inst);
        break;
    case MUTEX_UNLOCK:
        mostrar_parametros(inst, inst->cant_parametros);
        mutex_unlock(inst);
        break;
    case THREAD_EXIT:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_exit(inst);
        break;
    case PROCESS_EXIT:
        mostrar_parametros(inst, inst->cant_parametros);
        process_exit(inst);
        break;
    case NO_RECONOCIDO:
        log_info(logger_cpu, "se leyo mal el identificador");
        ejecutando_un_proceso = false;
        break;
    default:
        break;
    }
    return inst;
}

// INSTRUCCIONES

// SET (Registro, Valor): Asigna al registro el valor pasado como parámetro
void set(instruccion *inst)
{
    uint32_t *dir_registro = get_direccion_registro(list_get(inst->parametros, 0));
    *dir_registro = atoi(list_get(inst->parametros, 1));
}

// SUM (Registro Destino, Registro Origen): Suma al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.
void sum(instruccion *inst)
{
    char *registro_destino = list_get(inst->parametros, 0);
    char *registro_origen = list_get(inst->parametros, 1);

    uint32_t *registro_des = get_direccion_registro(registro_destino); // direccion del registro destino
    uint32_t *registro_ori = get_direccion_registro(registro_origen);  // dreccion del registro origen

    uint32_t aux = *registro_des + *registro_ori;

    *registro_des = aux;
}

// SUB (Registro Destino, Registro Origen): Resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.
void sub(instruccion *inst)
{
    char *registro_destino = list_get(inst->parametros, 0);
    char *registro_origen = list_get(inst->parametros, 1);

    uint32_t *registro_des = get_direccion_registro(registro_destino); // direccion del registro destino
    uint32_t *registro_ori = get_direccion_registro(registro_origen);  // direccion del registro origen

    uint32_t aux = *registro_des - *registro_ori;

    *registro_des = aux;
}

// JNZ (Registro, Instrucción): Si el valor del registro es distinto de cero,
// actualiza el program counter al número de instrucción pasada por parámetro.
void jnz(instruccion *inst)
{
    uint32_t *dir_registro = get_direccion_registro(list_get(inst->parametros, 0));
    char *pcf = list_get(inst->parametros, 1);

    if (*dir_registro != 0)
    {
        int program_counter_futuro = atoi(pcf);
        registros_cpu.PC = program_counter_futuro;
    }
}

// READ_MEM (Registro Datos, Registro Dirección): Lee el valor de memoria correspondiente a la Dirección Lógica
// que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.
void read_mem(instruccion *inst)
{
}

// WRITE_MEM (Registro Dirección, Registro Datos): Lee el valor del Registro Datos y lo escribe en la dirección
// física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
void write_mem(instruccion *inst)
{
    uint32_t *dir_registro = get_direccion_registro(list_get(inst->parametros, 0));
    uint32_t *datos = get_direccion_registro(list_get(inst->parametros, 1));

    uint32_t dir_fisica = traducir_direcciones(registros_cpu, *dir_registro);
    if (dir_fisica == -1)
    {
        devolver_contexto_de_ejecucion(pid, tid);
        t_paquete *paquete = crear_paquete(SEGMENTATION_FAULT);
        agregar_a_paquete(paquete, &tid, sizeof(uint32_t));
        enviar_paquete(paquete, conexion_dispatch);
        eliminar_paquete(paquete);
        return -1;
    }
    
    uint32_t tamanio = calcular_bytes(registros_cpu, dir_fisica);


    t_paquete *paquete_write = crear_paquete(WRITE_MEM);
    agregar_a_paquete_solo(paquete_write, &tid, sizeof(uint32_t));
    agregar_a_paquete_solo(paquete_write, &dir_fisica, sizeof(uint32_t));
    agregar_a_paquete_solo(paquete_write, datos, sizeof(uint32_t));
    agregar_a_paquete_solo(paquete_write, &tamanio, sizeof(uint32_t));
    enviar_paquete(paquete_write, socket_memoria);
    eliminar_paquete(paquete_write);

    recibir_mensaje(socket_memoria, logger_cpu);

    log_info(logger_cpu, "TID: <%i> - Acción: <ESCRIBIR> - Dirección Física: <%i>", tid, dir_fisica);
}

// LOG (Registro): Escribe en el archivo de log el valor del registro.
void log_instruccion(instruccion *inst)
{
    char *registro = list_get(inst->parametros, 0);
    uint32_t *valor_registro = get_direccion_registro(registro);

    log_info(logger_cpu, "El valor del registro es %i", *valor_registro);
}

// SYSCALLS
// MANDAR PARAMETROS A KERNEL Y CONTEXTO A MEMORIA

void dump_memory(instruccion *inst)
{
    devolver_lista_instrucciones(DUMP_MEMORY, inst);
}

void io(instruccion *inst)
{
    devolver_lista_instrucciones(IO, inst);
}

void process_create(instruccion *inst)
{
    devolver_lista_instrucciones(PROCESS_CREATE, inst);
}

void thread_create(instruccion *inst)
{
    devolver_lista_instrucciones(THREAD_CREATE, inst);
}

void thread_join(instruccion *inst)
{
    devolver_lista_instrucciones(THREAD_JOIN, inst);
}

void thread_cancel(instruccion *inst)
{

    devolver_lista_instrucciones(THREAD_CANCEL, inst);
}

void mutex_create(instruccion *inst)
{

    devolver_lista_instrucciones(MUTEX_CREATE, inst);
}

void mutex_lock(instruccion *inst)
{
    devolver_lista_instrucciones(MUTEX_LOCK, inst);
}

void mutex_unlock(instruccion *inst)
{
    devolver_lista_instrucciones(MUTEX_UNLOCK, inst);
}

void thread_exit(instruccion *inst)
{
    devolver_lista_instrucciones(THREAD_EXIT, inst);
}

void process_exit(instruccion *inst)
{
    devolver_lista_instrucciones(PROCESS_EXIT, inst);
}
/* ------------------------------------------- MMU ------------------------------------------- */

uint32_t traducir_direcciones(t_registros_cpu registros_cpu, uint32_t dir_logica)
{
    uint32_t dir_fisica;

    if (dir_logica <= registros_cpu.limite)
    {
        dir_fisica = registros_cpu.base + dir_logica;
        return dir_fisica;
    }
    else
    {
        return -1; // VER ESTO
    }
}

uint32_t calcular_bytes(t_registros_cpu registros_cpu, uint32_t dir_fisica)
{
    uint32_t tamanio;
    int limite_restante = registros_cpu.limite - dir_fisica;
    if (limite_restante >= 4)
    {
        tamanio = 4;
    }
    else
    {
        tamanio = limite_restante;
    }
    return tamanio;
}